/**
 * @file HcalMipTriggerProducer.cxx
 * @brief Implementaiton of HcalMipTriggerProducer class
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "Hcal/HcalMipTriggerProducer.h"

namespace ldmx {

    void HcalMipTriggerProducer::configure(const ldmx::ParameterSet& ps) {
        
        hitCollName_ = ps.getString( "HcalHitCollectionName" );
        hitPassName_ = ps.getString( "HcalHitPassName" );
        
        int nBackLayers = ps.getInteger( "NumLayersBackHcal" );
        nLayersPerOrientation_[ 0 ] = nBackLayers/2;
        nLayersPerOrientation_[ 1 ] = nBackLayers/2;
        if ( nBackLayers % 2 == 1 ) { //odd number back layers
            nLayersPerOrientation_[ 1 ] ++;
        }
        nLayersPerOrientation_[ 2 ] = ps.getInteger( "NumLayersTopHcal" );
        nLayersPerOrientation_[ 3 ] = ps.getInteger( "NumLayersBottomHcal" );
        nLayersPerOrientation_[ 4 ] = ps.getInteger( "NumLayersLeftHcal" );
        nLayersPerOrientation_[ 5 ] = ps.getInteger( "NumLayersRightHcal" );

        trackRadius_ = ps.getDouble( "TrackRadius" );

        minFracLayersHit_ = ps.getDouble( "MinFractionLayersHit" );
        
        absoluteMinHits_ = ps.getInteger( "AbsoluteMinNumberHits" );

        maxEnergy_ = ps.getDouble( "MaximumEnergy" );

        minPE_ = ps.getDouble( "MinimumPE" );

        triggerObjectName_ = ps.getString( "HcalMipTriggerObjectName" );

        numPass_ = 0;

        return;
    }

    void HcalMipTriggerProducer::produce(ldmx::Event& event) {
        
        //initialize event containers
        for ( int iO = 0; iO < 6; iO++ ) {
            hitLog_[ iO ].clear();
        }
        
        //obtain list of raw hits
        const TClonesArray *rawhits = event.getCollection( hitCollName_ , hitPassName_ );

        //add only plausible mip hits to hitLog_
        for ( size_t iH = 0; iH < rawhits->GetEntriesFast(); iH++ ) {
            ldmx::HcalHit* chit = (ldmx::HcalHit*)(rawhits->At(iH));
            if ( isPlausibleMip( chit ) ) { //could be a MIP
                
                int corient = chit->getSection();
                int clayer = chit->getLayer();
                
                //The underlying ints for the HcalSection and HcalOrientation enums are shifted by one
                // except for EVEN layers in the BACK Hcal.
                if ( corient != HcalSection::BACK or clayer % 2 == 1 ) {
                    corient++;
                }
                
                HitLogNode cNode;
                cNode.layer = clayer;
                cNode.strip = chit->getStrip();
                cNode.isGood = true;
                hitLog_[ corient ][ chit->getID() ] = cNode;
            } //could be a MIP
        } //iterate through rawhits (iH)
        
        //Find (and count) tracks
        int trackcnt = 0;
        for ( int corient = 0; corient < 6; corient++ ) {
            
            //while good end points are being found
            while ( findEndPoints( corient ) ) {
                
                //track to be constructed
                std::vector< unsigned int > track;
                
                int hitcnt = 0; //will not double count hits in same layer (if shallow) or same strip (if steep)
                int startLayer = (startPt_->second).layer;
                int startStrip = (startPt_->second).strip;
                int dstrip = ( (finishPt_->second).strip - startStrip );
                int dlayer = ( (finishPt_->second).layer - startLayer );
                
                if ( abs(dlayer) > 1 ) {
                    //non-steep slope, shallow angle muon
                    
                    //calculate slope
                    float slope = static_cast<float>(dstrip)/static_cast<float>(dlayer);
                    
                    //count hits in this orientation that are in the cylinder
                    std::set<int> countedLayers; //layers that already have been counted
                    for ( auto node : hitLog_[ corient ] ) {
                        
                        int cstrip = (node.second).strip;
                        int clayer = (node.second).layer;
    
                        float trackstrip = slope*( clayer - startLayer ) + startStrip;
    
                        float stripdif = std::abs( trackstrip - cstrip );
    
                        if ( stripdif < trackRadius_ ) {
                            //hit is in track cylinder
                            if ( countedLayers.find( clayer ) == countedLayers.end() ) {
                                hitcnt++;
                                countedLayers.insert( clayer );
                            }
    
                            track.push_back( node.first ); 
                        } //check if hit is in track cylinder
    
                    } //iterate through hitLog of current orientation (node)
                
                } else if ( abs(dstrip) > 1 ) {
                    //steep slope, check along strips instead

                    //calculate slope
                    float slope = static_cast<float>(dlayer)/static_cast<float>(dstrip);

                    //count hits in this orientation that are in the track cylinder
                    std::set<int> countedStrips; //strips that have already been counted
                    for ( auto node : hitLog_[ corient ] ) {
                        
                        int cstrip = (node.second).strip;
                        int clayer = (node.second).layer;

                        float tracklayer = slope*( cstrip - startStrip ) + startLayer;

                        float layerdif = std::abs( tracklayer - clayer );

                        if ( layerdif < trackRadius_ ) {
                            //hit is in track cylinder
                            if ( countedStrips.find( cstrip ) == countedStrips.end() ) {
                                hitcnt++;
                                countedStrips.insert( cstrip );
                            }

                            track.push_back( node.first );
                         } //check if hit is in the track cylinder

                    } //iterate through hitLog of current orientation (node)
                
                } //steep/shallow slope
                //skips end points if -1 <= dlayer <= 1 and -1 <= dstrip <= 1

                if ( hitcnt > minFracLayersHit_*hitLog_[ corient ].size() ) {
                    //good track found
                    
                    for ( unsigned int rawID : track ) {
                        hitLog_[ corient ].erase( rawID );
                    } //iterate through track (rawID)
                    
                    trackcnt++;

                } else {
                    //mark the end points as not good end points
                    (startPt_->second).isGood = false;
                    (finishPt_->second).isGood = false;
                } //what to do if track is good
            
            } //while good end points are still being found

        } //for each orientation (corient)
        
        bool pass = false;
        if ( trackcnt > 0 ) { //result_.getNumTracks() > 0 ) {
            pass = true;
            numPass_++;
        }

        result_.set( triggerObjectName_ , pass , 5 );
        result_.setAlgoVar( 0 , minPE_ );
        result_.setAlgoVar( 1 , maxEnergy_ );
        result_.setAlgoVar( 2 , minFracLayersHit_ );
        result_.setAlgoVar( 3 , trackRadius_ );
        result_.setAlgoVar( 4 , static_cast<double>( trackcnt ) );

        event.addToCollection( "Trigger" , result_ );
        
        numTracksPerEvent_[ trackcnt ] ++;

        return;
    }
    
    void HcalMipTriggerProducer::onProcessEnd() {
        
        printf( "\n" );
        printf( " ============================================\n" );
        printf( " |HcalMipTriggerProducer - Performance Stats|\n" );
        printf( " |==========================================|\n" );
        printf( " |          Num Passed : %-19d|\n" , numPass_ );
        printf( " |==========================================|\n" );
        printf( " |            N Tracks : N Events           |\n" );
        for ( auto track_event : numTracksPerEvent_ ) {
            printf( " |%20d : %-19d|\n" , track_event.first , track_event.second );
        }
        printf( " ============================================\n" );
        
        return;
    }

    bool HcalMipTriggerProducer::isPlausibleMip( ldmx::HcalHit* hit ) const {
        return ( hit->getPE() > minPE_ and hit->getEnergy() < maxEnergy_ );
    }

    bool HcalMipTriggerProducer::findEndPoints( int orientation ) {
        
        startPt_ = hitLog_[ orientation ].end();
        finishPt_ = hitLog_[ orientation ].end();

        if ( hitLog_[ orientation ].size() >= absoluteMinHits_ ) {
            
            for ( std::map< unsigned int , HitLogNode >::iterator it = hitLog_[ orientation ].begin();
                it != hitLog_[ orientation ].end(); ++it ) {

                if ( (it->second).isGood ) {
                
                    int clayer = (it->second).layer;
                    int cstrip = (it->second).strip;
    
                    if ( startPt_ == hitLog_[ orientation ].end() //haven't chosen a starting point yet
                         or clayer < (startPt_->second).layer //lower layer
                         or (clayer == (startPt_->second).layer and cstrip < (startPt_->second).strip) //same layer but lower strip
                       ) {
                        startPt_ = it;
                    } //if it->second could be a good start point
    
                    if ( finishPt_ == hitLog_[ orientation ].end() //haven't chosen a finishing point yet
                         or clayer > (finishPt_->second).layer //higher layer
                         or (clayer == (finishPt_->second).layer and cstrip > (finishPt_->second).strip) //same layer but higher strip
                       ) {
                        finishPt_ = it;
                    } //if it->second could be a good finish point
                
                } //is current hit good

            } //iterate through hits in this orientation hitlog (node)
        
        } //two ore more hits in this part of the log

        return ( startPt_ != hitLog_[ orientation ].end() and finishPt_ != hitLog_[ orientation ].end() );

    }

}

DECLARE_PRODUCER_NS( ldmx , HcalMipTriggerProducer );
