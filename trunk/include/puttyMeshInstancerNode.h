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

 
#ifndef __puttyMeshInstancer_h__
#define __puttyMeshInstancer_h__


// Maya
#include <maya/MGlobal.h>

#include <maya/MPxLocatorNode.h>
#include <maya/MTypeId.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MFnArrayAttrsData.h>
#include <maya/MMatrixArray.h>
#include <vector.h>


#define LEAD_COLOR				18	// green
#define ACTIVE_COLOR			15	// white
#define ACTIVE_AFFECTED_COLOR	8	// purple
#define DORMANT_COLOR			4	// blue
#define HILITE_COLOR			17	// pale blue

#define ROT_UNIT_DEG	0
#define ROT_UNIT_RAD	1

/////////////////////////////////////////////////////////////////////
//
// Shape class - defines the non-UI part of a shape node
//
class puttyMeshInstancer : public MPxLocatorNode
{
	public:

	puttyMeshInstancer();
	virtual ~puttyMeshInstancer(); 

	virtual void postConstructor();

	virtual MStatus getVectorArray( MFnArrayAttrsData &particleFn, const MString vectorName, MVectorArray &vectorArray, bool &exists );                                             
	virtual MStatus getDoubleArray( MFnArrayAttrsData &particleFn, const MString doubleName, MDoubleArray &doubleArray, bool &exists );                                                 
	virtual MStatus computeInstanceData( const MPlug&, MDataBlock& block);    
	virtual MStatus computeDisplayLists( const MPlug&, MDataBlock& block);    
	virtual MStatus compute( const MPlug&, MDataBlock& block);
    
	virtual void drawInstancesShaded();    
	virtual void drawInstancesWireframe(M3dView & view, M3dView::DisplayStatus status);        
    virtual void draw( M3dView & view, const MDagPath & path, M3dView::DisplayStyle style,  M3dView::DisplayStatus status );
    

   	virtual bool isBounded() const;
	virtual MBoundingBox boundingBox() const;
	virtual bool isTransparent() const; 

	static  void *		creator();
	static  MStatus		initialize();


	// Attributes    
	static	MTypeId	id;    
    static  MObject aParticle; // particle input
	static  MObject aMesh; // input for meshes to instance    
    static  MObject aDisplayListsReady; // is everything prepared for drawing?
    static  MObject aInstanceDataReady; // is everything prepared for drawing?    
  	
	static MObject aRotationUnit; // degrees or
	static MObject aObjectId; // degrees or    
            
	    static MObject aColorR; 
        static MObject aColorG; 
	    static MObject aColorB; 
    static MObject aColor; 

    static MObject aOpacity; 

    private:
    virtual void	deleteDisplayLists();
   	virtual MStatus	buildDisplayList(MObject &meshObj, GLuint id );
    MIntArray mapObjectIdToDisplayList;    
 
 	MVectorArray instancePosition;	// list of all instance positions
	MVectorArray instanceRotation;    // rotations
	MVectorArray instanceScale; 	// scale
	MDoubleArray instanceVisibility;     
	MIntArray 	 instanceDisplayList;         
    
	MVectorArray instanceColor; 
    
    float instanceOpacity;
    int instanceCount;
    
	bool instanceWireframeOverShaded;    
       	
};



#endif
