//
// plugin.cpp, de/register all nodes with maya
//

#include <maya/MFnPlugin.h>
#include "../include/helperMacros.h"
#include "../include/puttyDeformerNode.h"
#include "../include/puttyFieldNode.h"
#include "../include/puttyParticleAttributeMapperNode.h"


MStatus initializePlugin( MObject obj )
{ 
	MStatus   status;
	MFnPlugin plugin( obj, "Carsten Kolve", "6.5", "Any");
	MGlobal::displayInfo("----------------------------------------------------------------------");
	MGlobal::displayInfo("puttyNodes 0.1                               (c) Carsten Kolve, 2006");
	MGlobal::displayInfo("Free for educational and private use. Charity ware for commercial use!");
	MGlobal::displayInfo("Visit www.kolve.com for news, updates and information on the licenses!");
	MGlobal::displayInfo("-----------------------------------------------------------------------");
    
    // nodes
    
    status = plugin.registerNode( "puttyDeformer", puttyDeformer::id, &puttyDeformer::creator, &puttyDeformer::initialize, MPxNode::kDeformerNode );
    SYS_ERROR_CHECK(status, "registering deformer node 'puttyDeformer' failed!");

	status = plugin.registerNode( "puttyField", puttyField::id, &puttyField::creator, &puttyField::initialize,MPxNode::kFieldNode );
    SYS_ERROR_CHECK(status, "registering field node 'puttyField' failed!");
        
	status = plugin.registerNode( "puttyMapper", puttyParticleAttributeMapper::id, &puttyParticleAttributeMapper::creator, &puttyParticleAttributeMapper::initialize,MPxNode::kParticleAttributeMapperNode );
    SYS_ERROR_CHECK(status, "registering  node 'puttyMapper' failed!");

	return status;
}

MStatus uninitializePlugin( MObject obj )
{
	MStatus   status;
	MFnPlugin plugin( obj );

    
    // nodes
    status = plugin.deregisterNode( puttyDeformer::id );
    SYS_ERROR_CHECK(status, "deregistering deformer node 'puttyDeformer' failed!");

    status = plugin.deregisterNode( puttyField::id );
    SYS_ERROR_CHECK(status, "deregistering field node 'puttyField' failed!");

    status = plugin.deregisterNode( puttyParticleAttributeMapper::id );
    SYS_ERROR_CHECK(status, "deregistering  node 'puttyParticleAttributeMapper' failed!");

	return status;
}

