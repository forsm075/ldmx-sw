/**
 * @file MipCluster.h
 * @author Tom Eichlersmith, University of Minnesota
 * @brief Header file for class MipCluster
 */

#ifndef HCAL_MIPCLUSTER_H
#define HCAL_MIPCLUSTER_H

//Standard Libraries
#include <vector> //for storage vector of HcalHit pointers

//ROOT Libs

//LDMX Framework
#include "DetDescr/HcalID.h" //for the HcalSection enum
#include "Hcal/HcalDetectorGeometry.h" //to calculate real space coordinates

namespace ldmx {
    
    /**
     * @class MipCluster
     * @brief Stores pointers to HcalHits that are considered a single MIP hit (usually due to proximity).
     */
    class MipCluster {
        public:
            /**
             * Default Constructor
             */
            MipCluster();

            /**
             * Add an HcalHit to the MipCluster
             */
            void addHit( const HcalHit* hit );
            
            /**
             * Merges the input cluster into this cluster
             */
            void mergeCluster( const MipCluster &cluster );

            /**
             * Re-calculate member variables that depend on the hits.
             * This should be called whenever the hcalHits vector changes.
             */
            void set();
            
            /**
             * Get the total energy of the MipCluster
             */
            float getEnergy() const { return totalEnergy_; }

            /**
             * Get the number of HcalHits in this MipCluster
             */
            int getNumHits() const { return hcalHits_.size(); }

            /**
             * Get vector of HcalHits in this MipCluster
             */
            const std::vector< const HcalHit* > &getHcalHits() const { return hcalHits_; }

        private:
            
            /**
             * Calculate the total energy of the MipCluster.
             */
            void setTotalEnergy();

            /**
             * Calculate the real space point and errors of the MipCluster.
             */
            void setRealPoint();
            
            /** Class instance to help calculate real space coordinates */
            static HcalDetectorGeometry hdg_;

            /** The total energy of the MipCluster */
            float totalEnergy_;
 
            /** Storage vector of pointers to HcalHits */
            std::vector< const HcalHit* > hcalHits_;

            /** Real Space point representing cluster */
            std::vector< float > point_;

            /** "Error" (more like uncertainty) in each coordinate of the point */
            std::vector< float > errs_;

    };

}

#endif /* HCAL_MIPCLUSTER_H */
