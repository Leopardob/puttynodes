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
MObject puttyGlyph::aScale; 

MObject puttyGlyph::aLineWidth; 
MObject puttyGlyph::aPointSize; 
MObject puttyGlyph::aDrawLines; 
MObject puttyGlyph::aDrawPoint1; 
MObject puttyGlyph::aDrawPoint2; 

MObject puttyGlyph::aLineColorR; 
MObject puttyGlyph::aLineColorG; 
MObject puttyGlyph::aLineColorB; 
MObject puttyGlyph::aLineColor; 

MObject puttyGlyph::aPointColorR; 
MObject puttyGlyph::aPointColorG; 
MObject puttyGlyph::aPointColorB; 
MObject puttyGlyph::aPointColor; 
    



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
    MFnTypedAttribute  	tAttr;
    MFnEnumAttribute   	eAttr;    
    MFnNumericAttribute nAttr;        

    // the particle object
    
    // type
   	aGlyphType = eAttr.create("glyphType", "gt", PG_GT_VECTOR, &status);
	eAttr.addField("vector (v1 pos, v2 dir)",PG_GT_VECTOR);        
	eAttr.addField("line (v1, v2 pos)",PG_GT_LINE);    
	eAttr.addField("bounding box (v1, v2 corner)",PG_GT_BBOX);
   	eAttr.setKeyable(true);
	eAttr.setStorable(true);
	SYS_ERROR_CHECK( addAttribute( aGlyphType ), "adding aGlyphType" );
    
	aScale =  nAttr.create( "scale", "sc", MFnNumericData::kFloat );
	nAttr.setDefault( 1.0 ); 
   	nAttr.setSoftMin( -5.0 ); 
   	nAttr.setSoftMax( 5.0 ); 
	nAttr.setKeyable( true ); 
	nAttr.setReadable( true ); 
	nAttr.setWritable( true ); 
	nAttr.setStorable( true ); 
    SYS_ERROR_CHECK( addAttribute( aScale ), "adding aScale" );   
    


	// display options


    aDrawLines = nAttr.create("drawLines", "dl", MFnNumericData::kBoolean, true);
    SYS_ERROR_CHECK( addAttribute( aDrawLines ), "adding aDrawLines" ); 

    aDrawPoint1 = nAttr.create("drawPoint1", "dp1", MFnNumericData::kBoolean, true);
    SYS_ERROR_CHECK( addAttribute( aDrawPoint1 ), "adding aDrawPoint1" ); 

    aDrawPoint2 = nAttr.create("drawPoint2", "dp2", MFnNumericData::kBoolean, true);
    SYS_ERROR_CHECK( addAttribute( aDrawPoint2 ), "adding aDrawPoint2" ); 
    
    
    	aLineColorR = nAttr.create( "lineColorR", "lcolr", MFnNumericData::kFloat );
        aLineColorG = nAttr.create( "lineColorG", "lcolg", MFnNumericData::kFloat );
        aLineColorB = nAttr.create( "lineColorB", "lcolb", MFnNumericData::kFloat );        
	aLineColor = nAttr.create( "lineColor", "lcol", aLineColorR, aLineColorG, aLineColorB);
    nAttr.setDefault(1.0,0.0,0.0);
 	nAttr.setStorable( true );
 	nAttr.setUsedAsColor( true );
    SYS_ERROR_CHECK ( addAttribute( aLineColor ), "adding aLineColor" );   


    	aPointColorR = nAttr.create( "pointColorR", "pcolr", MFnNumericData::kFloat);
        aPointColorG = nAttr.create( "pointColorG", "pcolg", MFnNumericData::kFloat);
        aPointColorB = nAttr.create( "pointColorB", "pcolb", MFnNumericData::kFloat);        
	aPointColor = nAttr.create( "pointColor", "pcol", aPointColorR, aPointColorG, aPointColorB);
    nAttr.setDefault(1.0,1.0,1.0);
 	nAttr.setStorable( true );
 	nAttr.setUsedAsColor( true );
    SYS_ERROR_CHECK ( addAttribute( aPointColor ), "adding aPointColor" );   

	aLineWidth =  nAttr.create( "lineWidth", "lw", MFnNumericData::kFloat );
	nAttr.setDefault( 1.0 ); 
   	nAttr.setMin( 0.0 ); 
   	nAttr.setSoftMax( 5.0 ); 
	nAttr.setKeyable( true ); 
	nAttr.setReadable( true ); 
	nAttr.setWritable( true ); 
	nAttr.setStorable( true ); 
    SYS_ERROR_CHECK( addAttribute( aLineWidth ), "adding aLineWidth" );   
    
	aPointSize =  nAttr.create( "pointSize", "ps", MFnNumericData::kFloat );
	nAttr.setDefault( 1.0 ); 
   	nAttr.setMin( 0.0 ); 
   	nAttr.setSoftMax( 5.0 ); 
	nAttr.setKeyable( true ); 
	nAttr.setReadable( true ); 
	nAttr.setWritable( true ); 
	nAttr.setStorable( true ); 
    SYS_ERROR_CHECK( addAttribute( aPointSize ), "adding aPointSize" );   
    
    

    // inputs
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
// transparency, maya 8
//bool puttyGlyph::isTransparent( ) const { return false; }


/*************************************************************************************/
// draw bounding box
void puttyGlyph::drawBBox(MVectorArray &vecIn1,MVectorArray &vecIn2)
{
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
}

/*************************************************************************************/
// draw line
void puttyGlyph::drawLine(MVectorArray &vecIn1,MVectorArray &vecIn2)
{
	glBegin(GL_LINES);
    
	for (int i=0;i<vecIn1.length();i++)
    {
		glVertex3f(vecIn1[i].x,vecIn1[i].y,vecIn1[i].z);
		glVertex3f(vecIn2[i].x,vecIn2[i].y,vecIn2[i].z);        
    }        
    
    glEnd();
}

/*************************************************************************************/
// draw vector
void puttyGlyph::drawVector(MVectorArray &vecIn1,MVectorArray &vecIn2, float scale)
{
	glBegin(GL_LINES);
    
	for (int i=0;i<vecIn1.length();i++)
    {
		glVertex3f(vecIn1[i].x,vecIn1[i].y,vecIn1[i].z);

        MVector pos2 = vecIn1[i] + scale * vecIn2[i];
        
		glVertex3f(pos2.x,pos2.y,pos2.z);        
    }        
    
    glEnd();
}



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
    
    float scale;
    plug.setAttribute(aScale);
    plug.getValue(scale);
    
    ///////////////////////////////////////////////////////////
    // drawing parameters
    
    plug.setAttribute(aDrawLines);
    bool mDrawLines; plug.getValue(mDrawLines);
      
    plug.setAttribute(aDrawPoint1);
    bool mDrawPoint1; plug.getValue(mDrawPoint1);

    plug.setAttribute(aDrawPoint2);
    bool mDrawPoint2; plug.getValue(mDrawPoint2);
    
    plug.setAttribute(aPointSize);
    float mPointSize; plug.getValue(mPointSize);

    plug.setAttribute(aLineWidth);
    float mLineWidth; plug.getValue(mLineWidth);

    // color
    plug.setAttribute(aLineColorR);
    float mLineColorR; plug.getValue(mLineColorR);

    plug.setAttribute(aLineColorG);
    float mLineColorG; plug.getValue(mLineColorG);

    plug.setAttribute(aLineColorB);
    float mLineColorB; plug.getValue(mLineColorB);
    
    plug.setAttribute(aPointColorR);
    float mPointColorR; plug.getValue(mPointColorR);

    plug.setAttribute(aPointColorG);
    float mPointColorG; plug.getValue(mPointColorG);

    plug.setAttribute(aPointColorB);
    float mPointColorB; plug.getValue(mPointColorB);    
      
    
    /////////////////////////////////////////////////////////////////////
    // verify the values
    
    if (vecIn1.length() != vecIn2.length())
    	return;

    /////////////////////////////////////////////////////////////////////
    // draw stuff
	
	view.beginGL();             
	

    glPushAttrib( GL_CURRENT_BIT );
    glMatrixMode (GL_MODELVIEW);

	// draw the lines
	if (mDrawLines)
    {
    
        float lw;
        glGetFloatv(GL_LINE_WIDTH,&lw);
	    glLineWidth(mLineWidth);

	   	glColor3f(mLineColorR,mLineColorG,mLineColorB);
    
	    switch(type)
    	{
    		case (PG_GT_BBOX): { drawBBox(vecIn1,vecIn2); break; }
	    	case (PG_GT_LINE): { drawLine(vecIn1,vecIn2); break; }
    		case (PG_GT_VECTOR): { drawVector(vecIn1,vecIn2,scale); break; }        
	    }
        
       	glLineWidth(lw);
    }
    
    
    
	// draw the points
    glColor3f(mPointColorR,mPointColorG,mPointColorB);
    
    float ps;
    glGetFloatv(GL_POINT_SIZE,&ps); 
//    glPointSize(mPointSize);

	glBegin (GL_POINTS);

	if (mDrawPoint1)
		for (int i=0;i<vecIn1.length();i++)
        	glVertex3f(vecIn1[i].x,vecIn1[i].y,vecIn1[i].z);
            
	if (mDrawPoint2)
		for (int i=0;i<vecIn2.length();i++)
        	glVertex3f(vecIn2[i].x,vecIn2[i].y,vecIn2[i].z);

	glEnd();
    
//    glPointSize(ps);
	glPopAttrib();
    
    
    // done
    
	view.endGL();
    

}

