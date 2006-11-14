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



#define checkError(status,msg)\
	if ( status != MStatus::kSuccess ) \
    {\
		MString fullMsg = status.errorString() + " -> " + msg;\
		printMsg( fullMsg.asChar() );\
        return status; \
	}


/*************************************************************/

MTypeId puttyMeshInstancer::id( PUTTYMESHINSTANCER_NODE_ID );

     
MObject puttyMeshInstancer::aParticle;   // particle input

/*************************************************************************************/

puttyMeshInstancer:: puttyMeshInstancer() {}
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

    // the particle object
    aParticle = tAttr.create( "particle", "p", MFnData::kDynArrayAttrs);
    tAttr.setStorable(true);
    tAttr.setKeyable(false);
	SYS_ERROR_CHECK( addAttribute( aParticle ), "adding aParticle" );  

	return MS::kSuccess;
}

/*************************************************************************************/
//MStatus	puttyMeshInstancer::compute( const MPlug& plug, MDataBlock& block )
//{
//	MStatus status;
//	return MS::kUnknownParameter;
//}
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
    
    /////////////////////////////////////////////////////////////////////
    // Get the values needed for drawing
	//
    
	MObject thisNode = thisMObject();
    
    MPlug plug(thisNode, aParticle);
	
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
/*
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
    return MS::kSuccess;
    
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
    
	view.beginGL(); 

	// Push the color settings
 // glPushAttrib( GL_COLOR_BUFFER_BIT | GL_CURRENT_BIT | GL_ENABLE_BIT | GL_PIXEL_MODE_BIT ); 
    glPointSize(3.0);
	
    glPushAttrib( GL_CURRENT_BIT );
	glBegin( GL_POINTS );


    
    for (int i=0;i<position.length();i++)
    {
		glVertex3f(position[i].x,position[i].y,position[i].z);
    }
	
        
    glEnd();
    
	// pop color bit
	glPopAttrib();

    // end drawing
	view.endGL();

}

