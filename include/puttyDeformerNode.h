// * @file puttyDeformer.h
// * @author Carsten Kolve
 
#ifndef __puttyDeformer_h__
#define __puttyDeformer_h__


// Maya
#include <maya/MGlobal.h>
#include <maya/MPxDeformerNode.h>
#include <maya/MItGeometry.h>
#include <maya/MTypeId.h>



// enum defines

#define MSD_SPACE_OBJECT    1
#define MSD_SPACE_WORLD     2

#define MSD_WEIGHTS_AUTO    1
#define MSD_WEIGHTS_USER    2

#define MSD_ENVELOPE_AUTO    1
#define MSD_ENVELOPE_USER    2


class puttyDeformer : public MPxDeformerNode
{
	public:
				puttyDeformer();
		virtual	~puttyDeformer();
		static	void*	creator();
		static	MStatus	initialize();
        virtual MStatus setDependentsDirty( const MPlug &plugBeingDirtied, MPlugArray &affectedPlugs );
        virtual MStatus compute( const MPlug& plug, MDataBlock& block )  ;     
		virtual MStatus deform( MDataBlock& data,
                                MItGeometry& iter,
	    					    const MMatrix& mat,
		   					    unsigned int multiIndex);			
	
		static  MTypeId id;
        
        static  MObject aScript;            // name of command to use for the deformation
		static  MObject aCmdBaseName;
        static  MObject aSource;            // flag indicating a resourcing of the script
		static  MObject aNodeReady;         // flag indicating the node is ready for computation
		static  MObject aScriptSourced;     // flag indicating the script is sourced
        static  MObject aDynDirty;          // helper flag used to pass on dirty information for dynamic attributes
        

        static  MObject aDefSpace;          // which space is the deformer working in
        static  MObject aDefWeights;        // deformer weights
        static  MObject aDefEnvelope;       // envelope

        // current values - they are a storage position for values that can be used by the script
        static  MObject aCurrMultiIndex;    // the current multi index
        static  MObject aCurrPosition;      // the current position
        static  MObject aCurrWeight;        // the current weight        
        static  MObject aCurrWorldMatrix;   // the current world matrix
//        static  MObject aCurrGeometryName;    // current object name
        static  MObject aCurrGeometryType;  // current object type           
};



#endif
