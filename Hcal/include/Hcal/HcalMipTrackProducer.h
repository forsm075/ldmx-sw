/**
 * @file HcalMipTrackProducer.h
 * @brief Header file for HcalMipTrackProducer class
 * @author Tom Eichlersmith, University of Minnesota
 */

#ifndef HCAL_HCALMIPTRACKPRODUCER_H
#define HCAL_HCALMIPTRACKPRODUCER_H

//STL
#include <string> //names of collections
#include <map> //cluster and hit logs
#include <iterator> //std::prev
#include <cmath> //sqrt and abs
#include <iostream> //cerr and cout
#include <ctime> //timing produce function

//LDMX Framework
#include "Event/Event.h"
#include "Framework/EventProcessor.h" //Needed to declare processor
#include "Framework/ParameterSet.h" // Needed to import parameters from configuration file
#include "Event/EventConstants.h" //for HcalMipTrack string
#include "Event/HcalMipTrack.h" //mip track container
#include "Hcal/HcalDetectorGeometry.h" //calculating real space coordinates
#include "Hcal/MipCluster.h" //cluster object

namespace ldmx {
    
    /**
     * @class HcalMipTrackProducer
     * @brief Producer that reconstructs MIP tracks through the hcal.
     *
     * @note VERY SLOW. Optimization still under way, currently takes
     *  ~3s PER EVENT to produce the tracks.
     */
    class HcalMipTrackProducer : public ldmx::Producer {
        public:

            HcalMipTrackProducer(const std::string& name, ldmx::Process& process) : ldmx::Producer(name, process) { }

            virtual void configure(const ldmx::ParameterSet& ps);

            virtual void produce(ldmx::Event& event);

            virtual void onFileOpen() { }

            virtual void onFileClose() { }

            virtual void onProcessStart() { } 

            virtual void onProcessEnd() {
                
                printf( "\n==========================================\n" );
                printf( "|HcalMipTrackProducer - Performance Stats|\n");
                printf( "|========================================|\n");
                printf( "|Mean Time produce    : %-10.8fs      |\n", meanTime_produce_/1000 );
                printf( "|Mean Log Touches     : %-10.2f       |\n" , meanNumTouchLogs_ );
                printf( "|========================================|\n" );
                printf( "|          N Tracks : N Events           |\n" );
                for ( auto keyvalpair : numTracksPerEvent_ ) {
                    printf( "|%18d : %-19d|\n" , keyvalpair.first , keyvalpair.second );
                }
                printf( "==========================================\n" );

                return;
            }

        private:
            
            /**
             * Check if a single HcalHit is not noise.
             *
             * @param hit HcalHit to check
             * @return true if not noise
             */
            bool isNotNoise( HcalHit* hit ) const;

            /**
             * Check if cluster is a mip.
             *
             * @param cluster MipCluster to check
             * @bool true if a mip
             */
            bool isMip( const MipCluster* cluster ) const;

            /**
             * Cluster HcalHits into MipClusters.
             *
             * @note Currently, this only clusters hits in the same section,layer. One can update this
             *  to cluster across layers.
             */
            void clusterHits();
            
            /**
             * Finds a seed to construct a track from.
             *
             * @return true if successfully found a seed
             */
            bool findSeed(  );

            /**
             * Finds best track out of the clusters that are left.
             * 
             * @param track_mipids vector of cluster ids in track that is found
             * @return true if track was found
             */
            bool buildTrack( std::vector< unsigned int > &track_mipids );

            /**
             * Determine if line (defined by origin and direction) hits the box defined by minimum and maximum corners.
             * Assumes all inputs are formatted correctly.
             * This algorithm is the Fast Ray-Box Intersection Algorithm developed by Andrew Woo
             *  from "Graphics Gems", Academic Press, 1990.
             * This algorithm returns false if ray points away from box (even if the extended line would intersect it),
             *  so it is suggested to project the ray back to a plane one one side of all boxes and then use an origin there.
             *
             * @param origin originating point of ray
             * @param dir direction vector of ray
             * @param minBox minimum corner of box
             * @param maxBox maximum corner of box
             * @return true if ray hits box
             */
            bool rayHitBox( const std::vector<double> origin , const std::vector<double> dir , 
                             const std::vector<double> minBox , const std::vector<double> maxBox ) const;
            
            /** Name of collection of HcalHits */
            std::string hcalHitCollName_;

            /** Name of pass to get HcalHits collection from */
            std::string hcalHitPassName_;

            /** Container for reconstructed tracks */
            TClonesArray *hcalMipTracks_;

            /** Collection name for event bus */
            std::string hcalMipTracksCollName_;
            
            /** Maximum number of tracks to be allowed, prevents infinite looping */
            int maxTrackCount_;

            /** Minimum number of PE to be considered not noise */
            double minPE_;

            /** Maximum energy of a cluster to be considered a mip */
            double maxEnergy_;

            /** Minimum number of clusters in track to be considered for ranking */
            int minNumClusters_;
            
            /** Geometry class instance to calculate transformation between detector id and real space */
            static HcalDetectorGeometry hdg_;

            /** Log of HcalHits, sorted by section,layer,strip information */
            std::map< unsigned int , HcalHit* > hcalHitLog_;

            /** Log of clusters still being considered for track */
            std::map< unsigned int , MipCluster > clusterLog_;

            /** Seed point */
            std::vector<double> seedPoint_;

            /** Errors aroudn seed point */
            std::vector<double> seedErrors_;

            /** Seed Mip ID */
            unsigned int seedID_;

            /** Map of MipCluster IDs that are good/bad seeds */
            std::map< unsigned int , bool > isGoodSeed_;

            // PERFORMANCE TRACKERS

            /** Record the number of events that have a certain number of tracks */
            std::map< int , int > numTracksPerEvent_;

            /** Record the mean time it takes to produce the tracks */
            double meanTime_produce_;
            
            /** Number of accesses to both logs in current event */
            unsigned long int numTouchLogs_;

            /** Number of accesses to both logs */
            double meanNumTouchLogs_;
    };
}

#endif /* HCAL_HCALMIPTRACKPRODUCER_H */
