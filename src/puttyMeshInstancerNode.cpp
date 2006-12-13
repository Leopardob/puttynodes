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


#include <maya/MFnMesh.h>

#include <maya/MPointArray.h>
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
MObject puttyMeshInstancer::aDisplayListsReady;
MObject puttyMeshInstancer::aMesh; 
MObject puttyMeshInstancer::aInstanceDataReady;
  	
	MObject puttyMeshInstancer::aColorR; 
	MObject puttyMeshInstancer::aColorG; 
	MObject puttyMeshInstancer::aColorB; 
MObject puttyMeshInstancer::aColor; 

MObject puttyMeshInstancer::aOpacity; 



/*************************************************************************************/
// constructor
puttyMeshInstancer:: puttyMeshInstancer() 
{
	mMeshDL = glGenLists(1);
    instanceCount = 0;
}

/*************************************************************************************/
// destructor
puttyMeshInstancer::~puttyMeshInstancer() 
{
}

/*************************************************************************************/

void puttyMeshInstancer::postConstructor()
{
	// ensure correct naming
  	MFnDependencyNode nodeFn(thisMObject());
  	nodeFn.setName("puttyMeshInstancerShape#");
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

	for (int i=0;i<instanceCount;i++)    
		result.expand( instancePosition[i]);

    return result;
}

bool puttyMeshInstancer::isTransparent( ) const
{
	MObject thisNode = thisMObject(); 
	MPlug plug( thisNode, aOpacity ); 
	float value; 
	plug.getValue( value ); 
    if (value==0.0)
    	return true;
    else
		return false; 
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


        aColorR = nAttr.create( "colorR", "colr", MFnNumericData::kFloat, 1.0 );
        aColorG = nAttr.create( "colorG", "colg", MFnNumericData::kFloat, 0.0 );
        aColorB = nAttr.create( "colorB", "colb", MFnNumericData::kFloat, 0.0 );        
	aColor = nAttr.create( "color", "col", aColorR, aColorG, aColorB);
 	nAttr.setStorable( true );
 	nAttr.setUsedAsColor( true );
    SYS_ERROR_CHECK( addAttribute( aColor ), "adding aColor" );      

	aOpacity = nAttr.create( "opacity", "opa", MFnNumericData::kFloat );
	nAttr.setDefault( 1.0 ); 
   	nAttr.setMin( 0.0 ); 
   	nAttr.setMax( 1.0 ); 
	nAttr.setKeyable( true ); 
	nAttr.setReadable( true ); 
	nAttr.setWritable( true ); 
	nAttr.setStorable( true ); 
    SYS_ERROR_CHECK( addAttribute( aOpacity ), "adding aOpacity" );      


    aDisplayListsReady = nAttr.create("displayListsReady", "dlr", MFnNumericData::kBoolean, false);
    nAttr.setStorable(false) ;
    SYS_ERROR_CHECK( addAttribute( aDisplayListsReady ), "adding aDisplayListsReady" ); 

    aInstanceDataReady = nAttr.create("instanceDataReadt", "idr", MFnNumericData::kBoolean, false);
    nAttr.setStorable(false) ;
    SYS_ERROR_CHECK( addAttribute( aInstanceDataReady ), "adding aInstanceDataReady" ); 

	attributeAffects(aMesh, aDisplayListsReady );          
	attributeAffects(aParticle, aInstanceDataReady );          
	attributeAffects(aColor, aInstanceDataReady );              
    
	return MS::kSuccess;
}


/*************************************************************************************/
// build an opengl display list from a maya mesh
MStatus	puttyMeshInstancer::buildDisplayList(MObject &meshObj, GLuint id )
{
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
        
        // get all poly points
		MPointArray polyPoints;
     	polyIter.getPoints(polyPoints,MSpace::kWorld);
		
        // get the number of triangles
   	    int numTriangles;
       	polyIter.numTriangles ( numTriangles );

		bool drawGlLine = true;
        glBegin(GL_TRIANGLES);		
        	
			// get all triangles of the poly                
			for (int i=0;i<numTriangles;i++)
      		{
				MPointArray points;
        	    MIntArray vertexId;
       	    	polyIter.getTriangle(i,points,vertexId,MSpace::kWorld);

       		    glNormal3f(normal.x,  normal.y, normal.z);   
                                                	
                glVertex3f(points[0].x, points[0].y, points[0].z);  
	    		glVertex3f(points[1].x, points[1].y, points[1].z);  
		    	glVertex3f(points[2].x, points[2].y, points[2].z);  
			}    
            
		glEnd();			            
            
	}            

    glEndList();
        

	return MS::kSuccess;
}

/*************************************************************************************/
// build the opengl display lists for all connected meshes
MStatus	puttyMeshInstancer::computeDisplayLists( const MPlug&, MDataBlock& block)
{
	MStatus status = MS::kSuccess;
    bool readyForDrawing = false;
    
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
    MDataHandle ready = block.outputValue(aDisplayListsReady);
	ready.set(readyForDrawing);        	            
    ready.setClean();
    
    return status;    
}


/*************************************************************************************/
// function to extract vector array information from an arrayAttr
MStatus puttyMeshInstancer::getVectorArray(	MFnArrayAttrsData &particleFn, 
                                            const MString vectorName,
                                            MVectorArray &vectorArray,
                                            bool &exists )
{
	MStatus status;
    MFnArrayAttrsData::Type arrayType;


	exists = false;
    	
    if (particleFn.checkArrayExist(vectorName, arrayType,  &status))
    {
//    	cerr << "\nbla!"<<vectorName;
  		if (arrayType == MFnArrayAttrsData::kVectorArray)
        {
      		vectorArray = particleFn.vectorArray(vectorName, &status);
            exists = true;
//            cerr << " va";
	    }
/*        
  		if (arrayType == MFnArrayAttrsData::kDoubleArray)        
        {
           MDoubleArray da = particleFn.doubleArray(vectorName, &status);
           
            cerr << " da" << da.length() << " array" <<da;
         }

  		if (arrayType == MFnArrayAttrsData::kIntArray)        
            cerr << " ia";

  		if (arrayType == MFnArrayAttrsData::kStringArray)        
            cerr << " sa";

  		if (arrayType == MFnArrayAttrsData::kLast)        
            cerr << " last";
        
  		if (arrayType == MFnArrayAttrsData::kInvalid)        
            cerr << " invalid";

	}
	cerr <<"\ngetvec "<<vectorName<<"  "<<exists;
  */
    return status;    
}



/*************************************************************************************/
// this function extracts all the data needed for drawing from the data block
// and stores it in the internal data storage for the locator
MStatus	puttyMeshInstancer::computeInstanceData( const MPlug&, MDataBlock& block)
{
	MStatus status;
    bool mInstanceDataReady = false;
    
    // get color
	MDataHandle colorDH = block.inputValue(aColor);    
	MFloatVector color = colorDH.asFloatVector();
    
    // get object from the data handle
    MDataHandle particleDH = block.inputValue(aParticle);
	MObject particleObj = particleDH.data();

	// get functionSet
	MFnArrayAttrsData particleFn(particleObj,&status);

//	cerr<< "\ncomiin " <<	particleFn.list();
    
    if (status.error()) 
    	cerr<<"\ncan't get particle fn";
    else
    {
   		bool positionExists;
   		bool rotationExists;
   		bool scaleExists ;                
   		bool colorExists;                
                
        MFnArrayAttrsData::Type arrayType;

		// the position array is our reference, it is mandatory
        status = getVectorArray(particleFn, "position", instancePosition, positionExists);
		if (positionExists)
        {
       		instanceCount = instancePosition.length();                                           
			mInstanceDataReady = true;
            
	        status = getVectorArray(particleFn, "rotation", instanceRotation, rotationExists);
        	if (rotationExists)
            	mInstanceDataReady =  (instanceRotation.length() == instanceCount);
            else
            	instanceRotation = MVectorArray(instanceCount,MVector::zero);

	        if (mInstanceDataReady)
            {
            	status = getVectorArray(particleFn, "scale", instanceScale, scaleExists);
	        	if (scaleExists)
    	        	mInstanceDataReady =  (instanceScale.length() == instanceCount);
        	    else
            		instanceScale = MVectorArray(instanceCount,MVector(1,1,1));
			}
            
            if (mInstanceDataReady)
            {
//            	cerr <<"\ncolor!";
		        status = getVectorArray(particleFn, "rgbPP", instanceColor, colorExists);
    	    	if (colorExists)
                {
//                	cerr <<"exists << " << instanceColor.length() << " " << instanceCount;
        	    	mInstanceDataReady =  (instanceColor.length() == instanceCount);
                    
                    }
	            else
    	    		// get default value and build an array
        	    	instanceColor = MVectorArray(instanceCount,MVector(color.x,color.y,color.z));
                    
//				cerr << instanceColor;                    
	        }
       	}
	}

	// set output
    MDataHandle ready = block.outputValue(aInstanceDataReady);
	ready.set(mInstanceDataReady);        	            
    ready.setClean();

    return status; 
}

/*************************************************************************************/
MStatus	puttyMeshInstancer::compute( const MPlug& plug, MDataBlock& block )
{  
    if (plug == aDisplayListsReady)
    {
//    	cerr <<"\ncomp dl";
		return computeDisplayLists(plug, block);
        }
	else if (plug == aInstanceDataReady)
    {
//        cerr <<"\ncomp id";
    	return computeInstanceData(plug, block);
        }
	else
	    return MS::kUnknownParameter;
}


/*************************************************************************************/
// draw the instances
void puttyMeshInstancer::drawInstancesShaded()
{
	glClearDepth(1.0);
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glShadeModel(GL_SMOOTH);
//	glDepthMask( GL_FALSE );
	glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);        
	glEnable(GL_LIGHTING);   
	glEnable(GL_POLYGON_OFFSET_FILL);         
	glEnable(GL_CULL_FACE);

	glMatrixMode (GL_MODELVIEW);
    
    for (int i=0; i < instanceCount; i++)
   	{
    	glPushMatrix();
        GLfloat mat[] = {instanceColor[i].x,instanceColor[i].y,instanceColor[i].z, instanceOpacity};
    	glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE, mat);
	    glTranslatef(instancePosition[i].x, instancePosition[i].y, instancePosition[i].z);            

    	// call the list
   	    glCallList(mMeshDL);
   		glPopMatrix();
	}        
}  


void puttyMeshInstancer::drawInstancesWireframe( M3dView & view , M3dView::DisplayStatus displayStatus)
{
	glDisable(GL_LIGHTING);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);            

	if (instanceWireframeOverShaded)
    {
		// choose the color for the wireframe based on the display status
    	M3dView::ColorTable activeColorTable = M3dView::kActiveColors;
		M3dView::ColorTable dormantColorTable = M3dView::kDormantColors;

    	MColor col;
		switch ( displayStatus )
		{
			case M3dView::kLead 			: col = view.colorAtIndex( LEAD_COLOR, activeColorTable );	break;
			case M3dView::kActive 			: col = view.colorAtIndex( ACTIVE_COLOR, activeColorTable ); break;
			case M3dView::kActiveAffected 	: col = view.colorAtIndex( ACTIVE_AFFECTED_COLOR, activeColorTable ); break;
			case M3dView::kDormant 			: col = view.colorAtIndex( DORMANT_COLOR, dormantColorTable );	break;
			case M3dView::kHilite 			: col = view.colorAtIndex( HILITE_COLOR, activeColorTable ); break;
			default: break;
		}
            
	    glColor4f(col.r,col.g,col.b,instanceOpacity);
        
        for (int i=0; i < instanceCount; i++)
   		{
	    	glPushMatrix();
		    glTranslatef(instancePosition[i].x, instancePosition[i].y, instancePosition[i].z);            

    		// call the list
   	    	glCallList(mMeshDL);
	   		glPopMatrix();
		}        
    
    }
	else
    {
	    for (int i=0; i < instanceCount; i++)
   		{
	    	glPushMatrix();
    		glColor4f (instanceColor[i].x,instanceColor[i].y,instanceColor[i].z, instanceOpacity);
		    glTranslatef(instancePosition[i].x, instancePosition[i].y, instancePosition[i].z);            

    		// call the list
   	    	glCallList(mMeshDL);
	   		glPopMatrix();
		}        
    }
}    



/*************************************************************************************/
// drawing function of the locator
void puttyMeshInstancer::draw( M3dView & view, const MDagPath & path, M3dView::DisplayStyle style, M3dView::DisplayStatus status )
{ 	
//	cerr << "\n----------------draw-----------";
	///////////////////////////////////
	// get the plugs that cause all the data for drawing to be ready
    // if they are not, don't do a thing
    MObject thisObject = thisMObject();

	MPlug plug( thisObject, aDisplayListsReady );
    bool mDisplayListsReady; plug.getValue( mDisplayListsReady );
	
    if (!mDisplayListsReady)
    	return;
    
    plug.setAttribute(aInstanceDataReady);
	bool mInstanceDataReady; plug.getValue( mInstanceDataReady );    
    
    if (!mInstanceDataReady)
    	return;
    
    // get opacity
    plug.setAttribute(aOpacity);
	plug.getValue( instanceOpacity );    



	///////////////////////////////////    
	// all the data needed for drawing is in place,
    
   	view.beginGL(); 


	// get the current opengl states, for restoring them later
	// get the current polygon mode
//	int polygonMode[2];
//	glGetIntegerv(GL_POLYGON_MODE, polygonMode);    
    
//    GLboolean lightingMode;
//    glGetBooleanv(GL_LIGHTING, &lightingMode);    



	// set up the opengl based on the current style and status
//    glPushAttrib( GL_COLOR_BUFFER_BIT | GL_CURRENT_BIT | GL_ENABLE_BIT |  GL_PIXEL_MODE_BIT ); 
	glPushAttrib( GL_ALL_ATTRIB_BITS);

    if ( instanceOpacity < 1.0f ) 
   	{ 
   		glEnable( GL_BLEND );
    	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    }
                    
    
    // figure out what we need to draw based on display style and status
    bool drawShaded = ((style == M3dView::kFlatShaded) || ( style == M3dView::kGouraudShaded)); 
    instanceWireframeOverShaded = ((status == M3dView::kActive) || (status == M3dView::kLead) || (status == M3dView::kHilite));
	bool drawWireframe = ((style == M3dView::kWireFrame) || ( style == M3dView::kPoints) || instanceWireframeOverShaded);


	// now on to the drawing
	if (drawShaded)
	    drawInstancesShaded();    


	if (drawWireframe)
	    drawInstancesWireframe(view,status);
    

	// restore the opengl state
//    glPopAttrib();
// 	glPolygonMode(GL_FRONT, polygonMode[0]);
//	glPolygonMode(GL_BACK,  polygonMode[1]);

//    if (!lightingMode)
//    	glDisable(GL_LIGHTING);

	glPopAttrib();

	view.endGL(); 
    
}







/*************************************************************************************/
// draw
/*

  	
  	  
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



}

*/
