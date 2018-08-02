/**
 * @file MipCluster.cxx
 * @brief Implementation file for class MipCluster
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "Hcal/MipCluster.h"

namespace ldmx {

    MipCluster::MipCluster() { }

    void MipCluster::addHit( const HcalHit* hit ) {
        hcalHits_.push_back( hit );
        return;
    }

    void MipCluster::mergeCluster( const MipCluster &cluster ) {
        
        const std::vector< const HcalHit* > hcalHitsnew = cluster.getHcalHits();

        for ( const HcalHit* hit : hcalHitsnew ) {
            this->addHit( hit );
        }

        return;
    }

    void MipCluster::set() {
        
        setRealPoint();

        setTotalEnergy();

    }

    void MipCluster::setTotalEnergy() {
        totalEnergy_ = 0.0;
        for ( unsigned int i = 0; i < hcalHits_.size(); i++ ) {
            totalEnergy_ += hcalHits_[i]->getEnergy();
        } //iterate through list of hits (i)
        return;
    }

    void MipCluster::setRealPoint() {
        
        //hdg_ is meant to contain all relationship between detector coordinates (section, layer, strip)
        // and real space coordinates (x,y,z). This allows for there to be only one place where updates
        // to the detector geometry need to be input. Investigate the HcalDetectorGeoemetry class to see
        // about changing how the real space point is calculated from HcalHits.
        hdg_.transformDet2Real( hcalHits_ , point_ , errs_ );

        return;
    }
}
