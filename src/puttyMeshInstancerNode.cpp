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

#include <maya/MTime.h>
#include <maya/MPoint.h>
#include <maya/MPlugArray.h>
#include <maya/MVectorArray.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include "../include/helperMacros.h"
#include "../include/puttyNodeIds.h"
#include "../include/puttyMeshInstancerNode.h"



#define checkStatus(status,msg)\
	if ( status != MStatus::kSuccess ) \
    {\
		MString fullMsg = status.errorString() + " -> " + msg;\
		printMsg( fullMsg.asChar() );\
        return status; \
	}


/*************************************************************/

MTypeId puttyMeshInstancer::id( PUTTYMESHINSTANCER_NODE_ID );

     
MObject puttyMeshInstancer::aParticle;
MObject puttyMeshInstancer::aReadyForDrawing;
MObject puttyMeshInstancer::aMesh; 

/*************************************************************************************/

puttyMeshInstancer:: puttyMeshInstancer() 
{
	mMeshDL = glGenLists(1);
}

puttyMeshInstancer::~puttyMeshInstancer() {}

/*************************************************************************************/
void* puttyMeshInstancer::creator() 
{ 
	return new puttyMeshInstancer(); 
}

/*************************************************************************************/
MStatus puttyMeshInstancer::initialize()
{
	MStatus status;
    MFnTypedAttribute       tAttr;
    MFnNumericAttribute       nAttr;    

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


MStatus	puttyMeshInstancer::buildDisplayList(MItMeshPolygon &meshIt, GLuint id )
{
	
    // build the display list
    glNewList(id,GL_COMPILE);
  		
        glBegin(GL_QUADS);			
            // up to top            
            glNormal3f(0.0,  1.0f, 0.0f);  
		    glVertex3f(-0.5f,  0.0f, -0.5f);  
		    glVertex3f(0.5f,  0.0f, -0.5f);  
		    glVertex3f(0.5f,  0.0f, 0.5f);                          
		    glVertex3f(-0.5f,  0.0f, 0.5f);                          
		glEnd();			
	
    glEndList();
        

	return MS::kSuccess;
}


/*************************************************************************************/
MStatus	puttyMeshInstancer::compute( const MPlug& plug, MDataBlock& block )
{
	MStatus status = MS::kSuccess;
    
    if (plug == aReadyForDrawing)
    {
    	cerr << "\ncompute";
		// get meshes and build display lists from them
	    MDataHandle meshDH = block.inputValue(aMesh);
	    MObject meshObj = meshDH.asMesh();

//    	if (meshObj==MObject::kNullObj)
//        	return MS::kFailure;
    
	    MItMeshPolygon meshIt(meshObj,&status);          

//	    SYS_ERROR_CHECK(status,"error initializing MItMeshPolygon!");
        
     	status = buildDisplayList(meshIt, mMeshDL );
        
		// get 
        MDataHandle ready = block.outputValue(aReadyForDrawing);

		        		
        if (!status.error())
			ready.set(true);        	
        else
        	ready.set(false);        	
            
        ready.setClean();
        
        return status;    
    }
    
	return MS::kUnknownParameter;
}
/*************************************************************************************/

// ensure correct naming
void puttyMeshInstancer::postConstructor()
{
  MFnDependencyNode nodeFn(thisMObject());
  nodeFn.setName("puttyMeshInstancerShape#");
}

/*************************************************************************************/
// bounding box info
bool puttyMeshInstancer::isBounded() const
{ 
	return false;
}

/*************************************************************************************/
// transparency
bool puttyMeshInstancer::isTransparent( ) const
{
	return false; 
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
/*************************************************************************************/
// draw
void puttyMeshInstancer::draw( M3dView & view, const MDagPath & /*path*/, 
							 M3dView::DisplayStyle style,
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
    glMatrixMode (GL_MODELVIEW);


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

