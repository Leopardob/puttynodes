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




class puttyMeshInstancer : public MPxLocatorNode
{
	public:
				puttyMeshInstancer();
		virtual	~puttyMeshInstancer();
		static	void*	creator();
		static	MStatus	initialize();
        
		virtual	void	postConstructor();            
           
        virtual void draw( M3dView & view, const MDagPath & path, 
		        		   M3dView::DisplayStyle style,
						   M3dView::DisplayStatus status );

		virtual MStatus	compute( const MPlug& plug, MDataBlock& block );

       	virtual bool isBounded() const;
       	virtual bool isTransparent() const; 
       
//        virtual MStatus compute( const MPlug& plug, MDataBlock& block )  ;     
	
		static  MTypeId id;
        
        static  MObject aParticle; // particle input
        static  MObject aReadyForDrawing; // is everything prepared for drawing?
    	static  MObject aMesh; // input for meshes to instance
        
    private:
    
    	virtual MStatus	buildDisplayList(MItMeshPolygon &meshIt, GLuint id );
        
        GLuint mMeshDL;
        
};



#endif
