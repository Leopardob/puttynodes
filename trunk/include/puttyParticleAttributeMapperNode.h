// * @file puttyParticleAttributeMapper.h
// * @author Carsten Kolve
 
#ifndef __puttyParticleAttributeMapper_h__
#define __puttyParticleAttributeMapper_h__

// Maya
#include <maya/MGlobal.h>
#include <maya/MPxParticleAttributeMapperNode.h>
#include <maya/MTypeId.h>

class puttyParticleAttributeMapper : public MPxParticleAttributeMapperNode
{
	public:
				puttyParticleAttributeMapper();
		virtual	~puttyParticleAttributeMapper();
		static	void*	creator();
		static	MStatus	initialize();
//        virtual MStatus setDependentsDirty( const MPlug &plugBeingDirtied, MPlugArray &affectedPlugs );
        virtual MStatus compute( const MPlug& plug, MDataBlock& block )  ;     
		virtual MStatus	computeScriptSourced( const MPlug& plug, MDataBlock& block );
		virtual MStatus	computeNodeReady( const MPlug& plug, MDataBlock& block );        
	
		static  MTypeId id;
        
        static  MObject aScript;            // name of command to use for the deformation
		static  MObject aCmdBaseName;
        static  MObject aSource;            // flag indicating a resourcing of the script
		static  MObject aNodeReady;         // flag indicating the node is ready for computation
		static  MObject aScriptSourced;     // flag indicating the script is sourced       

		static	MObject	aParticleCount;
};

#endif
