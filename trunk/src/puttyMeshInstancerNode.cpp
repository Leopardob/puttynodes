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


// maya includes
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnArrayAttrsData.h>

#include <maya/MDrawData.h>
#include <maya/MDrawRequest.h>
#include <maya/MMaterial.h>
#include <maya/MSelectionMask.h>
#include <maya/MSelectionList.h>
#include <maya/MDagPath.h>
#include <maya/MFnMesh.h>
#include <maya/MMatrix.h>

#include <maya/MPointArray.h>
#include <maya/MPlugArray.h>
#include <maya/MVectorArray.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include "../include/helperMacros.h"
#include "../include/puttyNodeIds.h"
#include "../include/puttyMeshInstancerNode.h"



/*************************************************************/
/////////////////////////////////////////////////////////////////////
// SHAPE NODE IMPLEMENTATION
/////////////////////////////////////////////////////////////////////


MTypeId puttyMeshInstancer::id( PUTTYMESHINSTANCER_NODE_ID ); 

MObject puttyMeshInstancer::aParticle;
MObject puttyMeshInstancer::aReadyForDrawing;
MObject puttyMeshInstancer::aMesh; 

/*************************************************************************************/
// constructor
puttyMeshInstancer:: puttyMeshInstancer() 
{
	mMeshDL = glGenLists(1);

	fGeometry = new puttyInstancedMeshGeom;
	fGeometry->readyForDrawing	= false;
	fGeometry->meshDL			= mMeshDL;
   // fGeometry->position			= MVectorArray(1, MVector::zero);
    
}

/*************************************************************************************/
// destructor
puttyMeshInstancer::~puttyMeshInstancer() 
{
	delete fGeometry;
}

/*************************************************************************************/

void puttyMeshInstancer::postConstructor()
{
	// ensure correct naming
  	MFnDependencyNode nodeFn(thisMObject());
  	nodeFn.setName("puttyMeshInstancerShape#");
  
	// This call allows the shape to have shading groups assigned
  	setRenderable( true );
}


/*************************************************************************************/
void* puttyMeshInstancer::creator() 
{ 
	return new puttyMeshInstancer(); 
}

/*************************************************************************************/
// bounding box info
bool puttyMeshInstancer::isBounded() const
{ 
	return true;
}

// Returns the bounding box for the shape.
MBoundingBox puttyMeshInstancer::boundingBox() const
{
	MBoundingBox result;	
	puttyMeshInstancer* nonConstThis = const_cast <puttyMeshInstancer*> (this);
	puttyInstancedMeshGeom* geom = nonConstThis->geometry();

	MVectorArray position(geom->position);

	for (int i=0;i<position.length();i++)    
		result.expand( position[i]);

    return result;
}

/*************************************************************************************/
MStatus puttyMeshInstancer::initialize()
{
	MStatus status;
    MFnTypedAttribute     tAttr;
    MFnNumericAttribute   nAttr;    

    // the particle object
    aParticle = tAttr.create( "particleData", "pd", MFnData::kDynArrayAttrs);
	tAttr.setStorable(true);
    tAttr.setKeyable(false);
	SYS_ERROR_CHECK( addAttribute( aParticle ), "adding aParticle" ); 
    
   	aMesh = tAttr.create("mesh", "me", MFnData::kMesh);
   	tAttr.setStorable(true) ;
   	tAttr.setKeyable(false) ;
   	tAttr.setReadable(true) ;
   	tAttr.setWritable(true) ;
   	tAttr.setCached(false) ;
   	SYS_ERROR_CHECK( addAttribute(aMesh), "adding aMesh");

    aReadyForDrawing = nAttr.create("readyForDrawing", "rfd", MFnNumericData::kBoolean, false);
    nAttr.setStorable(false) ;
    SYS_ERROR_CHECK( addAttribute( aReadyForDrawing ), "adding aReadyForDrawing" ); 

	attributeAffects(aMesh, aReadyForDrawing );          

	return MS::kSuccess;
}


MStatus	puttyMeshInstancer::buildDisplayList(MObject &meshObj, GLuint id )
{
	// get the world space translate, this will be subtracted from everything to
    // position the instances correctly
    MStatus status;
    
    MItMeshPolygon polyIter(meshObj,&status);          
    SYS_ERROR_CHECK(status,"error initializing MItMeshPolygon!");	     


	
    // build the display list
    glNewList(id,GL_COMPILE);


    // iterate over all polys
	for ( ; !polyIter.isDone(); polyIter.next() )
	{
		// does the mesh have a valid triangulation?
        if (!polyIter.hasValidTriangulation())
        {
        	USER_ERROR_CHECK(MS::kFailure,"puttyMeshInstancer: the mesh does have bad geometry and no valid triangulation - do a cleanup first!");
        }
		
        // get the normal of the poly
		MVector normal;
        polyIter.getNormal(normal, MSpace::kWorld);


        // only use triangles, they should be most fast to draw
        	
    	// is the poly convex? if yes then we don't need to triangulate it and can use the
        // vertices straig away
/*        if(polyIter.isConvex())
        {
			// get all polygon vertices
			MPointArray points;
     	    polyIter.getPoints(points,MSpace::kWorld);

			// build the display list for the poly
	        glBegin(GL_POLYGON);			
      		    glNormal3f(normal.x,  normal.y, normal.z);  

				for (int i=0;i<points.length();i++)
		    		glVertex3f(points[i].x, points[i].y, points[i].z);  
				            
			glEnd();			
        }
        else
        {*/
        	// poly is concave, use the triangulation instead
	        // get the number of triangles
    	    int numTriangles;
        	polyIter.numTriangles ( numTriangles );

	        glBegin(GL_TRIANGLES);			          
				// get all triangles of the poly                
				for (int i=0;i<numTriangles;i++)
    	    	{
					MPointArray points;
    	    	    MIntArray vertexId;
        	    	polyIter.getTriangle(i,points,vertexId,MSpace::kWorld);
            
		    	    // add triangles to display list
        		    glNormal3f(normal.x,  normal.y, normal.z);  
		    		glVertex3f(points[0].x, points[0].y, points[0].z);  
		    		glVertex3f(points[1].x, points[1].y, points[1].z);  
			    	glVertex3f(points[2].x, points[2].y, points[2].z);  
	    		}    
			glEnd();			            
            
//       	}
	}            

    glEndList();
        

	return MS::kSuccess;
}


/*************************************************************************************/
MStatus	puttyMeshInstancer::compute( const MPlug& plug, MDataBlock& block )
{
	MStatus status = MS::kSuccess;
    
    if (plug == aReadyForDrawing)
    {
    	bool readyForDrawing = false;
        
        cerr << "\ncompute";
		// get meshes and build display lists from them
	    MDataHandle meshDH = block.inputValue(aMesh);
	    MObject meshObj = meshDH.asMesh();

    	if (meshObj != MObject::kNullObj)
        {
	     	status = buildDisplayList(meshObj, mMeshDL );
        	
            if (!status.error())
            	readyForDrawing = true;
		}
		        		
		// set output
       	MDataHandle ready = block.outputValue(aReadyForDrawing);
		ready.set(readyForDrawing);        	            
        ready.setClean();
        
        return status;    
    }
    
	return MS::kUnknownParameter;
}


/*************************************************************************************/
puttyInstancedMeshGeom* puttyMeshInstancer::geometry()
//
// This function gets the values of all the attributes and
// assigns them to the fGeometry. Calling MPlug::getValue
// will ensure that the values are up-to-date.
//
{
//	cerr <<"\n---------------------------interface obj!";
    
	// are we ready for drawing? this will also cause display lists to be build
    MObject this_object = thisMObject();
	MPlug plug( this_object, aReadyForDrawing );
    bool readyForDrawing;
    plug.getValue( readyForDrawing );
	
//	cerr <<"\ninterface ref! " << readyForDrawing;
    
   	// get the particle data
    plug.setAttribute(aParticle);
	
    // is the node connected? if not early exit
    MPlugArray connPlugs;
    if(plug.connectedTo(connPlugs,true,false))
    {
        if (connPlugs.length()==0)
            readyForDrawing = false;
    }
    else
    	readyForDrawing = false;


	MVectorArray position        ;
    
    // get object from the plug
    MObject particleData;
    MStatus status = plug.getValue(particleData); 
	if (status.error()) 
    {	
    	cerr<<"\ncan't get particle object";
        readyForDrawing = false;
    }
    else
    {
    
	    MFnArrayAttrsData particleFn(particleData,&status);
		
        if (status.error()) 
        {
        	cerr<<"\ncan't get particle fn";
            readyForDrawing = false;
        }    
        else
        {
    
		 	// variables to store stuff in
		//	MVectorArray position, rotation, scale, color;
		    int count=1;

//			cerr << "\npfn " << particleFn.list();

	    	// analyse what's in the array attrs        
    		MFnArrayAttrsData::Type arrayType;
    
		    if ((particleFn.checkArrayExist("position", arrayType,  &status)) && (arrayType == MFnArrayAttrsData::kVectorArray))
    		{
       			position = particleFn.vectorArray("position", &status);
		        if (position.length() >1)
        		{
		           	if (count==1)
        		    	count = position.length();
		            else
        		    	if (count != position.length())
							 readyForDrawing = false;
		        }
		    }  
		    else
		    {
       			// TODO use the default
		        position = MVectorArray(1,MVector::zero);
		    }
		}
	}
	// finally set all the values for the interface object
//    cerr <<"\ninterface rfd! "<<readyForDrawing;

        
    fGeometry->readyForDrawing = readyForDrawing;
    fGeometry->position.copy(position);  
    
//    cerr <<"\ninterface pos! "  << fGeometry->position;
      
    fGeometry->meshDL = mMeshDL;        


	return fGeometry;
}


/*************************************************************************************/
// function to extract vector array information from an arrayAttr
/*MStatus puttyMeshInstancer::getVectorArray(	MObject &thisNode, 
											MObject &attribute,
											MFnArrayAttrsData &particleFn, 
                                            const MString vectorName,
                                            MVectorArray vectorArray,
                                            int &count )
{
	MStatus status;
    MFnArrayAttrsData::Type arrayType;
    
    if ((particleFn.checkArrayExist(vectorName, arrayType,  &status)) && (arrayType == MFnArrayAttrsData::kVectorArray))
    {
       	vectorArray = particleFn.vectorArray(vectorName, &status);
        int length = vectorArray.length();
        if (length >1)
        {
           	if (count==1)
            	count = length;
            else
            	if (count != length)
					return MS::kFailure;
        }
    }
    return MS::kSuccess;    
}
*/


/////////////////////////////////////////////////////////////////////
// UI IMPLEMENTATION
/////////////////////////////////////////////////////////////////////


puttyMeshInstancerUI::puttyMeshInstancerUI() {}
puttyMeshInstancerUI::~puttyMeshInstancerUI() {}

void* puttyMeshInstancerUI::creator()
{
	return new puttyMeshInstancerUI();
}

/*************************************************************************************/
/* override */
void puttyMeshInstancerUI::getDrawRequests( const MDrawInfo & info,bool /*objectAndActiveOnly*/, MDrawRequestQueue & queue)
{
	// The draw data is used to pass geometry through the 
	// draw queue. The data should hold all the information
	// needed to draw the shape.
	//
	MDrawData data;
	MDrawRequest request = info.getPrototype( *this );
	puttyMeshInstancer* shapeNode = (puttyMeshInstancer*)surfaceShape();
	puttyInstancedMeshGeom* geom = shapeNode->geometry();
	getDrawData( geom, data );
	request.setDrawData( data );

	// Use display status to determine what color to draw the object
	//
	switch ( info.displayStyle() )
	{		 
		case M3dView::kWireFrame :
				request.setToken( kDrawWireframe );        
				getDrawRequestsWireframe( request, info );
				queue.add( request );
			break;

		case M3dView::kPoints :
				request.setToken( kDrawPoints );        
				getDrawRequestsWireframe( request, info );
				queue.add( request );
			break;
		
		case M3dView::kGouraudShaded :
				request.setToken( kDrawSmoothShaded );
				getDrawRequestsShaded( request, info, queue, data );
				queue.add( request );
			break;
		
		case M3dView::kFlatShaded :
				request.setToken( kDrawFlatShaded );
 				getDrawRequestsShaded( request, info, queue, data );
				queue.add( request );
			break;
            
            
		default:	
			break;
	}
}

/*************************************************************************************/
void puttyMeshInstancerUI::getDrawRequestsShaded( MDrawRequest& request,
											const MDrawInfo& info,
											MDrawRequestQueue& queue,
											MDrawData& data )
{
	cerr <<"\nrequest shaded";
    
	// Need to get the material info
	//
	MDagPath path = info.multiPath();	// path to your dag object 
	M3dView view = info.view();; 		// view to draw to
	MMaterial material = MPxSurfaceShapeUI::material( path );
	M3dView::DisplayStatus displayStatus = info.displayStatus();

	// Evaluate the material 
	if ( ! material.evaluateMaterial( view, path ) ) 
		cerr << "Couldnt evaluate\n";

	// material has texture?
	bool drawTexture = true;
	if ( drawTexture && material.materialIsTextured() ) 
		material.evaluateTexture( data );

	request.setMaterial( material );

	// material is transparent?
	bool materialTransparent = false;
	material.getHasTransparency( materialTransparent );
	if ( materialTransparent ) 
		request.setIsTransparent( true );

	
	// create a draw request for wireframe on shaded if
	// necessary.
	if ( (displayStatus == M3dView::kActive) || 
    	 (displayStatus == M3dView::kLead) || (displayStatus == M3dView::kHilite) )
	{
		MDrawRequest wireRequest = info.getPrototype( *this );
		wireRequest.setDrawData( data );
		getDrawRequestsWireframe( wireRequest, info );
		wireRequest.setToken( kDrawWireframeOnShaded );
		wireRequest.setDisplayStyle( M3dView::kWireFrame );
		queue.add( wireRequest );
	}
}

/*************************************************************************************/
void puttyMeshInstancerUI::getDrawRequestsWireframe( MDrawRequest& request, const MDrawInfo& info )
{
	// set the drawing color of the wireframe based on the display status

	M3dView::DisplayStatus displayStatus = info.displayStatus();
	M3dView::ColorTable activeColorTable = M3dView::kActiveColors;
	M3dView::ColorTable dormantColorTable = M3dView::kDormantColors;
	switch ( displayStatus )
	{
		case M3dView::kLead 			: request.setColor( LEAD_COLOR, activeColorTable );	break;
		case M3dView::kActive 			: request.setColor( ACTIVE_COLOR, activeColorTable ); break;
		case M3dView::kActiveAffected 	: request.setColor( ACTIVE_AFFECTED_COLOR, activeColorTable ); break;
		case M3dView::kDormant 			: request.setColor( DORMANT_COLOR, dormantColorTable );	break;
		case M3dView::kHilite 			: request.setColor( HILITE_COLOR, activeColorTable ); break;
		default: break;
	}
}

/*************************************************************************************/
/* override */
bool puttyMeshInstancerUI::select( MSelectInfo &selectInfo,
							 MSelectionList &selectionList,
							 MPointArray &worldSpaceSelectPts ) const
//
// Select function. Gets called when the bbox for the object is selected.
// This function just selects the object without doing any intersection tests.
//
{
/*	MSelectionMask priorityMask( MSelectionMask::kSelectObjectsMask );
	MSelectionList item;
	item.add( selectInfo.selectPath() );
	MPoint xformedPt;
	selectInfo.addSelection( item, xformedPt, selectionList,
							 worldSpaceSelectPts, priorityMask, false );
	*/
    return true;
}

/*************************************************************************************/

/* override */
void puttyMeshInstancerUI::draw( const MDrawRequest & request, M3dView & view ) const
//
// From the given draw request, get the draw data and determine what to draw
//
{ 	
//  	cerr <<"\n//////DRAW////////////////////";
	MDrawData data = request.drawData();
	
    // get the geometry
	puttyInstancedMeshGeom * geom = (puttyInstancedMeshGeom*)data.geometry();
	
    // are the display lists build? if not, don't bother drawing anything
    if (! geom->readyForDrawing)
    	return;


    MVectorArray pos(geom->position);
    
    
    
	bool drawTexture = false;
	short token = request.token();    
    
	view.beginGL(); 

	if ( (token == kDrawSmoothShaded) || (token == kDrawFlatShaded) )
	{
		glEnable( GL_POLYGON_OFFSET_FILL );

		// Set up the material
/*		MMaterial material = request.material();
		material.setMaterial( request.multiPath(), request.isTransparent() );

		// Enable texturing
		drawTexture = material.materialIsTextured();
		if ( drawTexture ) glEnable(GL_TEXTURE_2D);

		// Apply the texture to the current view
		//
		if ( drawTexture ) 
			material.applyTexture( view, data );
	*/
    }

	// get the current polygon mode
	int polygonMode[2];
	glGetIntegerv(GL_POLYGON_MODE, polygonMode);    
    

	switch( token )
	{
		case kDrawWireframe :
		case kDrawWireframeOnShaded :        
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);        
            break;
            
		case kDrawPoints :
			glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);        
            break;

//		case kDrawSmoothShaded : break;
//		case kDrawFlatShaded :	break;
	}


	// do the actual drawing
    	
	glMatrixMode (GL_MODELVIEW);
    
	glColor3f(0,1,0);
    for (int i=0; i < geom->position.length(); i++)
   	{
    	glPushMatrix();
    
	    glTranslatef(float(geom->position[i].x),
        			 float(geom->position[i].y),
                     float(geom->position[i].z));            
                     

    	// call the list
   	    glCallList(geom->meshDL);
        
   		glPopMatrix();
	}        

	// restore the polygon mode
 	glPolygonMode(GL_FRONT, polygonMode[0]);
	glPolygonMode(GL_BACK, polygonMode[1]);

    
	// Turn off texture mode
	if ( drawTexture ) glDisable(GL_TEXTURE_2D);

	view.endGL(); 
}








/*************************************************************************************/
// draw
/*
//void puttyMeshInstancer::draw( M3dView & view, const MDagPath & /*path*/
//, 
/*							 M3dView::DisplayStyle style,
							 M3dView::DisplayStatus stat )
{ 
    MStatus status;
    
    cerr <<"\ndraw--------------";
    /////////////////////////////////////////////////////////////////////
    // Get the values needed for drawing
    
	MObject thisNode = thisMObject();

	// get mReadyToDraw, this causes the displays lists to be created
    MPlug plug(thisNode, aReadyForDrawing);
    bool mReadyForDrawing; plug.getValue(mReadyForDrawing);
    
    if (!mReadyForDrawing) 
    	return;
	
        
    // get the particle data
    plug.setAttribute(aParticle);
	
    // is the node connected? if not early exit
    MPlugArray connPlugs;
    if(plug.connectedTo(connPlugs,true,false))
    {
        if (connPlugs.length()==0)
            return;
    }
    else
        return;
        
    // get object from the plug
    MObject particleData;
    status = plug.getValue(particleData); 
	if (status.error()) {cerr<<"\ncan't get particle object";return;}
    
    MFnArrayAttrsData particleFn(particleData,&status);
	if (status.error()) {cerr<<"\ncan't get particle fn";return;}    
    
 	// variables to store stuff in
	MVectorArray position, rotation, scale, color;
    int count;
    count =1;
  
  	
  	  
	cerr << "\npfn " << particleFn.list();
 
    // analyse what's in the array attrs        
    MFnArrayAttrsData::Type arrayType;
    
    if ((particleFn.checkArrayExist("position", arrayType,  &status)) && (arrayType == MFnArrayAttrsData::kVectorArray))
    {
       	position = particleFn.vectorArray("position", &status);
        if (position.length() >1)
        {
           	if (count==1)
            	count = position.length();
            else
            	if (count != position.length())
					return;
        }
    }  
    else
    {
       	// TODO use the default
        position = MVectorArray(1,MVector::zero);
    }

    if (particleFn.checkArrayExist("rotation", arrayType,  &status))
    	if (arrayType == MFnArrayAttrsData::kVectorArray)
        	rotation = particleFn.vectorArray("rotation", &status);

/*    if (particleFn.checkArrayExist("scale", arrayType,  &status))
    	if (arrayType == MFnArrayAttrsData::kVectorArray)
        	scale = particleFn.getVectorData("scale", &status);

    if (particleFn.checkArrayExist("color", arrayType,  &status))
    	if (arrayType == MFnArrayAttrsData::kVectorArray)
        	color = particleFn.getVectorData("color", &status);
*/


    /////////////////////////////////////////////////////////////////////    
    // do the actual drawing
	

//    glPushAttrib( GL_CURRENT_BIT );
/*    glMatrixMode (GL_MODELVIEW);


 	float lw;
	//glGetFloatv(GL_LINE_WIDTH,&lw);
	//glLineWidth(mLineWidth);

	if( (style == M3dView::kFlatShaded) || (style == M3dView::kGouraudShaded) ) 
    {
		glPushAttrib( GL_COLOR_BUFFER_BIT | GL_CURRENT_BIT | GL_ENABLE_BIT |  GL_PIXEL_MODE_BIT ); 

	    glColor4f(1,0,0,1);
    
		for (int i=0; i < position.length(); i++)
    	{
	    	glPushMatrix();
    
		    glTranslatef(float(position[i].x),float(position[i].y),float(position[i].z));            
	        //glScalef(fabs(diff.x),fabs(diff.y),fabs(diff.z));                

	    	// call the list
    	    glCallList(mMeshDL);
        
    		glPopMatrix();
		}        

    	// pop color bit
		glPopAttrib();
    }

    // end drawing
	view.endGL();

}

*/
