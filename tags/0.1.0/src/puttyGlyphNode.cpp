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

#include <maya/MPoint.h>
#include <maya/MVectorArray.h>
#include <maya/MFnVectorArrayData.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include "../include/helperMacros.h"
#include "../include/puttyNodeIds.h"
#include "../include/puttyGlyphNode.h"

#include "math.h"


/*************************************************************/

MTypeId puttyGlyph::id( PUTTYGLYPH_NODE_ID );
     
MObject puttyGlyph::aVecInput1;   
MObject puttyGlyph::aVecInput2;   
MObject puttyGlyph::aGlyphType;   

/*************************************************************************************/
// constructor, build all display lists required

puttyGlyph:: puttyGlyph() 
{
	//////////////////////////////////////////////////////////////////////////
	// build display list for box - box with l/w/d of 1 centered around origin
    mBBoxDL = glGenLists(1);
    
    glNewList(mBBoxDL,GL_COMPILE);
  		
        glBegin(GL_LINE_STRIP);			
        
			// Bottom Face
		    glVertex3f(-0.5f, -0.5f, -0.5f);  
		    glVertex3f( 0.5f, -0.5f, -0.5f);  
		    glVertex3f( 0.5f, -0.5f,  0.5f);  
		    glVertex3f(-0.5f, -0.5f,  0.5f);  
		    glVertex3f(-0.5f, -0.5f, -0.5f);  
            
            // up to top            
		    glVertex3f(-0.5f,  0.5f, -0.5f);  

			// top face
		    glVertex3f( 0.5f,  0.5f, -0.5f);  
		    glVertex3f( 0.5f,  0.5f,  0.5f);  
		    glVertex3f(-0.5f,  0.5f,  0.5f);  
		    glVertex3f(-0.5f,  0.5f, -0.5f);  

		glEnd();			
        
		// missing lines from bottom to top
        glBegin(GL_LINES);	    
		    glVertex3f( 0.5f, -0.5f, -0.5f);  
		    glVertex3f( 0.5f,  0.5f, -0.5f);              
            
		    glVertex3f( 0.5f, -0.5f,  0.5f);  
			glVertex3f( 0.5f,  0.5f,  0.5f);              
            
		    glVertex3f(-0.5f, -0.5f,  0.5f);  
		    glVertex3f(-0.5f,  0.5f,  0.5f);  
		glEnd();			
	
    glEndList();
}


puttyGlyph::~puttyGlyph() {}

/*************************************************************************************/
void* puttyGlyph::creator() 
{ 
	return new puttyGlyph(); 
}


/*************************************************************************************/
MStatus puttyGlyph::initialize()
{
	MStatus status;
    MFnTypedAttribute  tAttr;
    MFnEnumAttribute   eAttr;    

    // the particle object
    
    // def space
   	aGlyphType = eAttr.create("glyphType", "gt", PG_GT_BBOX, &status);
	eAttr.addField("bounding box",PG_GT_BBOX);
   	eAttr.setKeyable(true);
	eAttr.setStorable(true);
	SYS_ERROR_CHECK( addAttribute( aGlyphType ), "adding aGlyphType" );
    
    aVecInput1 = tAttr.create( "vecInput1", "vi1", MFnData::kVectorArray);
    tAttr.setStorable(false);
    tAttr.setKeyable(false);
	SYS_ERROR_CHECK( addAttribute( aVecInput1 ), "adding aVecInput1" );  

    aVecInput2 = tAttr.create( "vecInput2", "vi2", MFnData::kVectorArray);
    tAttr.setStorable(false);
    tAttr.setKeyable(false);
	SYS_ERROR_CHECK( addAttribute( aVecInput2 ), "adding aVecInput2" );  
    
    
	return MS::kSuccess;
}

/*************************************************************************************/
//MStatus	puttyGlyph::compute( const MPlug& plug, MDataBlock& block )
//{
//	MStatus status;
//	return MS::kUnknownParameter;
//}
/*************************************************************************************/

// ensure correct naming
void puttyGlyph::postConstructor()
{
  MFnDependencyNode nodeFn(thisMObject());
  nodeFn.setName("puttyGlyphShape#");
}

/*************************************************************************************/
// bounding box info
bool puttyGlyph::isBounded() const { return true; }

MBoundingBox puttyGlyph::boundingBox() const
{ 
	MBoundingBox bbox;
    MStatus status;
    
    // get inputs
    MVectorArray vecIn1, vecIn2;
    MObject vecObj;
    
    MPlug plug(thisMObject(), aVecInput1);
    plug.getValue(vecObj);
    MFnVectorArrayData vecFn1(vecObj);
    vecIn1 = vecFn1.array(&status);
    
    plug.setAttribute(aVecInput2);
    plug.getValue(vecObj);
    MFnVectorArrayData vecFn2(vecObj);
    vecIn2 = vecFn2.array(&status);

	// verify length
    if (vecIn1.length() == vecIn2.length())
    {	
    	//put all points in bbox;
        for (int i=0;i<vecIn1.length();i++)
        {
	        bbox.expand(vecIn1[i]);
	        bbox.expand(vecIn2[i]);            
         }
    }    
    
    return bbox;
}

/*************************************************************************************/
// transparency
bool puttyGlyph::isTransparent( ) const { return false; }

/*************************************************************************************/
// draw
void puttyGlyph::draw( M3dView & view, const MDagPath & /*path*/, 
							 M3dView::DisplayStyle style,
							 M3dView::DisplayStatus stat )
{ 
    MStatus status;
    
    /////////////////////////////////////////////////////////////////////
    // Get the values needed for drawing
    
	MObject thisNode = thisMObject();

    MPlug plug(thisNode, aGlyphType);
    
    short type;
    plug.getValue(type);
    
    MVectorArray vecIn1, vecIn2;
    MObject vecObj;
    
    plug.setAttribute(aVecInput1);
    plug.getValue(vecObj);
    MFnVectorArrayData vecFn1(vecObj);
    vecIn1 = vecFn1.array(&status);
    
    plug.setAttribute(aVecInput2);
    plug.getValue(vecObj);
    MFnVectorArrayData vecFn2(vecObj);
    vecIn2 = vecFn2.array(&status);
    
    /////////////////////////////////////////////////////////////////////
    // verify the values
    
    if (vecIn1.length() != vecIn2.length())
    	return;

    /////////////////////////////////////////////////////////////////////
    // draw stuff
	
	view.beginGL();             
	
    
    if (type == PG_GT_BBOX)
    {
        glPushAttrib( GL_CURRENT_BIT );
        glMatrixMode (GL_MODELVIEW);
        
        // color    
		if ( status == M3dView::kActive ) 
			view.setDrawColor( 13, M3dView::kActiveColors );
		else
			view.setDrawColor( 13, M3dView::kDormantColors );

	    for (int i=0;i<vecIn1.length();i++)
    	{
            glPushMatrix();
            
            // move to the correct spot, set up the right scale
            MVector diff = vecIn2[i] - vecIn1[i];        
            MVector pos = (vecIn1[i] + vecIn2[i]) * 0.5;
            
            glTranslatef(float(pos.x),float(pos.y),float(pos.z));            
	        glScalef(fabs(diff.x),fabs(diff.y),fabs(diff.z));                

            // call the list
            glCallList(mBBoxDL);
            
            glPopMatrix();
	    }        
        
    	glPopAttrib();
    }
	

	view.endGL();
    

}

