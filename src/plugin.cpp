/* COPYRIGHT --
 *
 * This file is part of puttyNodes, a collection of utility nodes for Autodesk Maya.
 * puttyNodes is (c) 2006 Carsten Kolve <carsten@kolve.com>
 * and distributed under the terms of the GNU GPL V2.
 * This contribution to puttyNodes is (c) Rising Sun Pictures PTY Ltd, www.rsp.com.au 
 * See the ./License-GPL.txt file in the source tree root for more information.
 *
 * THIS PACKAGE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES
 * OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

//
// plugin.cpp, de/register all nodes with maya
//

#include <maya/MFnPlugin.h>
#include "../include/helperMacros.h"
#include "../include/puttyDeformerNode.h"
#include "../include/puttyFieldNode.h"
#include "../include/puttyMapperNode.h"
#include "../include/puttyMeshInstancerNode.h"


MStatus initializePlugin( MObject obj )
{ 
	MStatus   status;
	MFnPlugin plugin( obj, "Carsten Kolve", "0.1", "Any");
	MGlobal::displayInfo("----------------------------------------------------------------------------------------");
	MGlobal::displayInfo("puttyNodes 0.1 (c) Carsten Kolve, 2006");
    MGlobal::displayInfo("Contributions to this plugin (c) Rising Sun Pictures PTY Ltd, www.rsp.com.au");
	MGlobal::displayInfo("Licensed under the GPL, if you find this useful please consider a donation to a charity!");
	MGlobal::displayInfo("Visit www.kolve.com for news, updates and information on the licenses!");
	MGlobal::displayInfo("----------------------------------------------------------------------------------------");
    
    // nodes
    
    status = plugin.registerNode( "puttyDeformer", puttyDeformer::id, &puttyDeformer::creator, &puttyDeformer::initialize, MPxNode::kDeformerNode );
    SYS_ERROR_CHECK(status, "registering deformer node 'puttyDeformer' failed!");

//	status = plugin.registerNode( "puttyField", puttyField::id, &puttyField::creator, &puttyField::initialize,MPxNode::kFieldNode );
//  SYS_ERROR_CHECK(status, "registering field node 'puttyField' failed!");
        
	status = plugin.registerNode( "puttyMapper", puttyMapper::id, &puttyMapper::creator, &puttyMapper::initialize,MPxNode::kParticleAttributeMapperNode );
    SYS_ERROR_CHECK(status, "registering  node 'puttyMapper' failed!");

//	status = plugin.registerNode( "puttyMeshInstancer", puttyMeshInstancer::id, &puttyMeshInstancer::creator, &puttyMeshInstancer::initialize,MPxNode::kLocatorNode );
//  SYS_ERROR_CHECK(status, "registering  node 'puttyMeshInstancer' failed!");

	return status;
}

MStatus uninitializePlugin( MObject obj )
{
	MStatus   status;
	MFnPlugin plugin( obj );

    
    // nodes
    status = plugin.deregisterNode( puttyDeformer::id );
    SYS_ERROR_CHECK(status, "deregistering deformer node 'puttyDeformer' failed!");

//  status = plugin.deregisterNode( puttyField::id );
//  SYS_ERROR_CHECK(status, "deregistering field node 'puttyField' failed!");

    status = plugin.deregisterNode( puttyMapper::id );
    SYS_ERROR_CHECK(status, "deregistering  node 'puttyMapper' failed!");

//  status = plugin.deregisterNode( puttyMeshInstancer::id );
//  SYS_ERROR_CHECK(status, "deregistering  node 'puttyMeshInstancer' failed!");


	return status;
}

