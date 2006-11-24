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

 
#ifndef __puttyGlyph_h__
#define __puttyGlyph_h__


// Maya
#include <maya/MGlobal.h>
#include <maya/MPxLocatorNode.h>
#include <maya/MTypeId.h>



#define PG_GT_BBOX 	 3
#define PG_GT_LINE 	 2
#define PG_GT_VECTOR 1




class puttyGlyph : public MPxLocatorNode
{
	public:
				puttyGlyph();
		virtual	~puttyGlyph();
		static	void*	creator();
		static	MStatus	initialize();
        
		virtual	void	postConstructor();            
        
		virtual void drawBBox(MVectorArray &vecIn1,MVectorArray &vecIn2);
		virtual void drawLine(MVectorArray &vecIn1,MVectorArray &vecIn2);
		virtual void drawVector(MVectorArray &vecIn1,MVectorArray &vecIn2,float scale);
                        
        virtual void draw( M3dView & view, const MDagPath & path, 
		        		   M3dView::DisplayStyle style,
						   M3dView::DisplayStatus status );

       	virtual bool isBounded() const;
		virtual MBoundingBox boundingBox() const;
//     	virtual bool isTransparent() const; // maya 8
       
	
		static  MTypeId id;
        
        static  MObject aGlyphType; // type
        static  MObject aVecInput1; // first vector array
        static  MObject aVecInput2; // 2nd vector array        


	    static MObject aLineWidth;
	    static MObject aPointSize;        

	    static MObject aDrawLines;
	    static MObject aDrawPoint1;        
	    static MObject aDrawPoint2;                
        
	        static MObject aLineColorR; 
    	    static MObject aLineColorG; 
	        static MObject aLineColorB; 
        static MObject aLineColor; 
  
  	        static MObject aPointColorR; 
    	    static MObject aPointColorG; 
	        static MObject aPointColorB; 
        static MObject aPointColor; 
  
  
        static  MObject aScale; // scale factor
        
	private:
    	
        GLuint mBBoxDL; // display list is for the bounding box
};



#endif
