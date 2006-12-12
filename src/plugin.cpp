/* COPYRIGHT --
 *
 * This file is part of puttyNodes, a collection of utility nodes for Autodesk Maya.
 * puttyNodes is (c) 2006 Carsten Kolve <carsten@kolve.com>
 * and distributed under the terms of the GNU GPL V2.
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
#include "../include/puttyMeshInstancerNode.h"
#include "../include/puttyGlyphNode.h"

#if MAYA_API_VERSION >= 800
	#include "../include/puttyMapperNode.h"
#endif // maya 8

MStatus initializePlugin( MObject obj )
{ 
	MStatus   status;
	MFnPlugin plugin( obj, "Carsten Kolve", "0.2", "Any");
	MGlobal::displayInfo("--------------------------------------------------------------------------------------");
	MGlobal::displayInfo("puttyNodes 0.2 (c) Carsten Kolve, 2006");
    MGlobal::displayInfo("Contributions to this plugin (c) Rising Sun Pictures PTY Ltd, www.rsp.com.au");
	MGlobal::displayInfo("Licensed under the GPL, if you find this useful please consider donating to a charity!");
	MGlobal::displayInfo("Visit www.kolve.com for news, updates and information on the licenses!");
	MGlobal::displayInfo("--------------------------------------------------------------------------------------");
    
    // nodes

	status = plugin.registerShape( "puttyMeshInstancer", puttyMeshInstancer::id, &puttyMeshInstancer::creator, &puttyMeshInstancer::initialize,&puttyMeshInstancerUI::creator );
  	SYS_ERROR_CHECK(status, "registering  node 'puttyMeshInstancer' failed!");

	status = plugin.registerNode( "puttyField", puttyField::id, &puttyField::creator, &puttyField::initialize,MPxNode::kFieldNode );
  	SYS_ERROR_CHECK(status, "registering field node 'puttyField' failed!");

    
    status = plugin.registerNode( "puttyDeformer", puttyDeformer::id, &puttyDeformer::creator, &puttyDeformer::initialize, MPxNode::kDeformerNode );
    SYS_ERROR_CHECK(status, "registering deformer node 'puttyDeformer' failed!");

	status = plugin.registerNode( "puttyGlyph", puttyGlyph::id, &puttyGlyph::creator, &puttyGlyph::initialize,MPxNode::kLocatorNode );
    SYS_ERROR_CHECK(status, "registering  node 'puttyGlyph' failed!");

#if MAYA_API_VERSION >= 800
	status = plugin.registerNode( "puttyMapper", puttyMapper::id, &puttyMapper::creator, &puttyMapper::initialize,MPxNode::kParticleAttributeMapperNode );
    SYS_ERROR_CHECK(status, "registering  node 'puttyMapper' failed!");
#endif // maya 8

	return status;
}

MStatus uninitializePlugin( MObject obj )
{
	MStatus   status;
	MFnPlugin plugin( obj );

	status = plugin.deregisterNode( puttyMeshInstancer::id );
//	cerr << status.errorString() << " bla "<< plugin.isNodeRegistered("puttyMeshInstancer");
	SYS_ERROR_CHECK(status, "deregistering  node 'puttyMeshInstancer' failed!");

	status = plugin.deregisterNode( puttyField::id );
  	SYS_ERROR_CHECK(status, "deregistering field node 'puttyField' failed!");

    status = plugin.deregisterNode( puttyDeformer::id );
    SYS_ERROR_CHECK(status, "deregistering deformer node 'puttyDeformer' failed!");

	status = plugin.deregisterNode( puttyGlyph::id );
	SYS_ERROR_CHECK(status, "deregistering  node 'puttyGlyph' failed!");

    
    // nodes
#if MAYA_API_VERSION >= 800
    status = plugin.deregisterNode( puttyMapper::id );
    SYS_ERROR_CHECK(status, "deregistering  node 'puttyMapper' failed!");
#endif // maya 8


	return status;
}

