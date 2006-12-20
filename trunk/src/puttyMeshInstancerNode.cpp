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
MObject puttyMeshInstancer::aRotationUnit;
  	
	MObject puttyMeshInstancer::aColorR; 
	MObject puttyMeshInstancer::aColorG; 
	MObject puttyMeshInstancer::aColorB; 
MObject puttyMeshInstancer::aColor; 

MObject puttyMeshInstancer::aOpacity; 



/*************************************************************************************/
// constructor
puttyMeshInstancer:: puttyMeshInstancer() 
{
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
   	tAttr.setArray(true) ;    
   	SYS_ERROR_CHECK( addAttribute(aMesh), "adding aMesh");

    MFnEnumAttribute eAttr;        
   
    // type
   	aRotationUnit = eAttr.create("rotationUnit", "ru", ROT_UNIT_RAD, &status);
	eAttr.addField("radians",ROT_UNIT_RAD);        
	eAttr.addField("degrees",ROT_UNIT_DEG);            
  	eAttr.setKeyable(true);
	eAttr.setStorable(true);
	SYS_ERROR_CHECK( addAttribute( aRotationUnit ), "adding aRotationUnit" );


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
	attributeAffects(aRotationUnit, aInstanceDataReady );              
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
    
	MArrayDataHandle hMeshArray = block.inputArrayValue( aMesh, &status);
	SYS_ERROR_CHECK(status, "ERROR in hMeshArray = block.inputArrayValue.\n");

	hMeshArray.jumpToElement(0);
    
    int count = hMeshArray.elementCount();
    meshDisplayLists = MIntArray(count);
    mapIdToDisplayList = MIntArray(count);
    
    for (int i=0;i< < hMeshArray.elementCount();i++)
    {
    	// get handles to children from the current array element
        MDataHandle hMeshElement = hMeshArray.inputValue(&status);
		SYS_ERROR_CHECK(status, "hMeshElement = hMeshArray.inputValue.\n");    
        
		MObject meshObj = hMeshElement.asMesh();

    	if (meshObj != MObject::kNullObj)
	    {
        	GLuint dlId = glGenLists(1);
            
		 	status = buildDisplayList(meshObj, dlId );
    	
        	if (status.error())
            {
            	readyForDrawing = false;
				break;
            }
            else
            {
	        	meshDisplayLists[i] = dlId;
                mapIdToDisplayList[i] = hMeshArray.currentIndex();
                readyForDrawing = true;
            }
		}
		
        hMeshArray.next();        
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

  */
  }
    return status;    
}

/*************************************************************************************/
// function to extract double array information from an arrayAttr
MStatus puttyMeshInstancer::getDoubleArray(	MFnArrayAttrsData &particleFn, 
                                            const MString doubleName,
                                            MDoubleArray &doubleArray,
                                            bool &exists )
{
	MStatus status;
    MFnArrayAttrsData::Type arrayType;


	exists = false;
    	
    if (particleFn.checkArrayExist(doubleName, arrayType,  &status))
    {
  		if (arrayType == MFnArrayAttrsData::kDoubleArray)
        {
      		doubleArray = particleFn.doubleArray(doubleName, &status);
            exists = true;

	    }
  	}
  
  	return status;    
}



/*************************************************************************************/
// this function extracts all the data needed for drawing from the data block
// and stores it in the internal data storage for the locator
MStatus	puttyMeshInstancer::computeInstanceData( const MPlug&, MDataBlock& block)
{
	MStatus status;
    bool mInstanceDataReady = false;
    
    // get rotationUnit  
	MDataHandle rotDH = block.inputValue(aRotationUnit);    
	short rotUnit = rotDH.asShort();
    
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
            {
            	mInstanceDataReady =  (instanceRotation.length() == instanceCount);
                
                //convert rotation from radians to degrees
                if (rotUnit == ROT_UNIT_RAD)
	                for (int i=0;i<instanceRotation.length();i++)
    	            	instanceRotation[i] *= 57.2957;
                
            }
            else
            	instanceRotation = MVectorArray(instanceCount,MVector::zero);

	        if (mInstanceDataReady)
            {
            	status = getVectorArray(particleFn, "scalePP", instanceScale, scaleExists);
	        	if (scaleExists)
    	        	mInstanceDataReady =  (instanceScale.length() == instanceCount);
        	    else
            		instanceScale = MVectorArray(instanceCount,MVector(1,1,1));
			}
            
            if (mInstanceDataReady)
            {
		        status = getVectorArray(particleFn, "rgbPP", instanceColor, colorExists);
    	    	if (colorExists)
                {
        	    	mInstanceDataReady =  (instanceColor.length() == instanceCount);
                }
	            else
    	    		// get default value and build an array
        	    	instanceColor = MVectorArray(instanceCount,MVector(color.x,color.y,color.z));
                    
//				cerr << instanceColor;                    
	        }
       	}
        
/*        // now put the whole shebang into a matrix array
        cerr << "\nmake mat";
        instanceMatrix.copy(MMatrixArray(instanceCount));
        cerr << "\nfill mat";
        for (int i=0;i<instanceCount;i++)
        {
	        MTransformationMatrix mat;
            double help[3];
            help[0] = instanceScale[i].x;
            help[1] = instanceScale[i].y;
            help[2] = instanceScale[i].z;
            
            mat.setScale(&instanceScale[i].x,MSpace::kWorld);

		    help[0] = instanceRotation[i].x;
            help[1] = instanceRotation[i].y;
            help[2] = instanceRotation[i].z;
                   
            mat.setRotation(help, MTransformationMatrix::kXYZ);
            mat.setTranslation(instancePosition[i], MSpace::kWorld);
            instanceMatrix[i] = mat.asMatrix();
        }
*/
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
        
//      glMultMatrixd( &(instanceMatrix[i].matrix[0][0]) );
        
	    glTranslated(instancePosition[i].x, instancePosition[i].y, 	instancePosition[i].z);                 
	    glRotated	(instanceRotation[i].z, 0.0,0.0,1.0);
		glRotated	(instanceRotation[i].y, 0.0,1.0,0.0);        
	    glRotated	(instanceRotation[i].x, 1.0,0.0,0.0);
        glScaled	(instanceScale[i].x, 	instanceScale[i].y, 	instanceScale[i].z);
        
                        
    	// call the list
   	    glCallList(mMeshDL);
   		glPopMatrix();
	}        
}  


void puttyMeshInstancer::drawInstancesWireframe( M3dView & view , M3dView::DisplayStatus displayStatus)
{
	glDisable(GL_LIGHTING);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);            

	// do we need to draw the wireframe only in one color (cause it's selected)
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
    
        	
	    	glTranslated(instancePosition[i].x, instancePosition[i].y, 	instancePosition[i].z);                 
		    glRotated	(instanceRotation[i].z, 0.0,0.0,1.0);
			glRotated	(instanceRotation[i].y, 0.0,1.0,0.0);        
	    	glRotated	(instanceRotation[i].x, 1.0,0.0,0.0);
	        glScaled	(instanceScale[i].x, 	instanceScale[i].y, 	instanceScale[i].z);


    		// call the list
   	    	glCallList(mMeshDL);
	   		glPopMatrix();
		}        
    
    }
	else
    {
    	// draw the wireframe using the pp  colors
	    for (int i=0; i < instanceCount; i++)
   		{
	    	glPushMatrix();
    		glColor4f (instanceColor[i].x,instanceColor[i].y,instanceColor[i].z, instanceOpacity);
	        
		    glTranslated(instancePosition[i].x, instancePosition[i].y, 	instancePosition[i].z);                 
	    	glRotated	(instanceRotation[i].z, 0.0,0.0,1.0);
			glRotated	(instanceRotation[i].y, 0.0,1.0,0.0);        
		    glRotated	(instanceRotation[i].x, 1.0,0.0,0.0);
        	glScaled	(instanceScale[i].x, 	instanceScale[i].y, 	instanceScale[i].z);



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
    
//   GLboolean lightingMode;
//   glGetBooleanv(GL_LIGHTING, &lightingMode);    

	// set up the opengl based on the current style and status
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
// 	glPolygonMode(GL_FRONT, polygonMode[0]);
//	glPolygonMode(GL_BACK,  polygonMode[1]);
//    if (!lightingMode)
//    	glDisable(GL_LIGHTING);

	glPopAttrib();

	view.endGL(); 
    
}


