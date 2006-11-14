// * @file puttyMeshInstancer.h
// * @author Carsten Kolve
 
#ifndef __puttyMeshInstancer_h__
#define __puttyMeshInstancer_h__


// Maya
#include <maya/MGlobal.h>
#include <maya/MPxLocatorNode.h>
#include <maya/MTypeId.h>




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

       	virtual bool isBounded() const;
       	virtual bool isTransparent() const; 
       
//        virtual MStatus compute( const MPlug& plug, MDataBlock& block )  ;     
	
		static  MTypeId id;
        
        static  MObject aParticle; // particle input
};



#endif
