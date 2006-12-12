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

#include <maya/MPxSurfaceShape.h>
#include <maya/MPxSurfaceShapeUI.h>
#include <maya/MTypeId.h>
#include <maya/MItMeshPolygon.h>


#define LEAD_COLOR				18	// green
#define ACTIVE_COLOR			15	// white
#define ACTIVE_AFFECTED_COLOR	8	// purple
#define DORMANT_COLOR			4	// blue
#define HILITE_COLOR			17	// pale blue

/////////////////////////////////////////////////////////////////////
// Geometry class, this will be our interface object between shape and ui
//
class puttyInstancedMeshGeom 
{
	public:
		MVectorArray position;
        GLuint meshDL;
        bool readyForDrawing;
//	MVectorArray rotation;
//	MVectorArray scale;        
//	MVectorArray color;            
};



/////////////////////////////////////////////////////////////////////
//
// Shape class - defines the non-UI part of a shape node
//
class puttyMeshInstancer : public MPxSurfaceShape
{
public:
	puttyMeshInstancer();
	virtual ~puttyMeshInstancer(); 

	virtual void			postConstructor();
	virtual MStatus			compute( const MPlug&, MDataBlock& );
//    virtual bool			getInternalValue( const MPlug&,
//											  MDataHandle& );
//    virtual bool			setInternalValue( const MPlug&,
//											  const MDataHandle& );
					  
	virtual bool            isBounded() const;
	virtual MBoundingBox    boundingBox() const; 

	static  void *		creator();
	static  MStatus		initialize();
	puttyInstancedMeshGeom*		geometry();

private:
	puttyInstancedMeshGeom*		fGeometry;

	// Attributes
	//
    
    static  MObject aParticle; // particle input
	static  MObject aMesh; // input for meshes to instance    
    static  MObject aReadyForDrawing; // is everything prepared for drawing?
  	
    
   	virtual MStatus	buildDisplayList(MObject &meshObj, GLuint id );
        
    GLuint mMeshDL;
 
public:
	static	MTypeId		id;
};

/////////////////////////////////////////////////////////////////////
//
// UI class	- defines the UI part of a shape node
//
class puttyMeshInstancerUI : public MPxSurfaceShapeUI
{
public:
	puttyMeshInstancerUI();
	virtual ~puttyMeshInstancerUI(); 

	virtual void	getDrawRequests( const MDrawInfo & info,
									 bool objectAndActiveOnly,
									 MDrawRequestQueue & requests );
                                     
	virtual void	draw( const MDrawRequest & request,
						  M3dView & view ) const;
                          
	virtual bool	select( MSelectInfo &selectInfo,
							MSelectionList &selectionList,
							MPointArray &worldSpaceSelectPts ) const;

	void			getDrawRequestsWireframe( MDrawRequest&,
											  const MDrawInfo& );
                                              
	void			getDrawRequestsShaded(	  MDrawRequest&,
											  const MDrawInfo&,
											  MDrawRequestQueue&,
											  MDrawData& data );

	static  void *  creator();

private:

	// Draw Tokens
	//
	enum {
		kDrawWireframe,
		kDrawWireframeOnShaded,
		kDrawSmoothShaded,
		kDrawFlatShaded,
        kDrawPoints,
		kLastToken
	};
};




#endif
