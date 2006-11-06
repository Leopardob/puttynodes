// * @author Carsten Kolve
 
#ifndef __puttyFieldNode_h__
#define __puttyFieldNode_h__


#include <maya/MIOStream.h>
#include <maya/MVector.h>
#include <maya/MObject.h>
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MPxFieldNode.h>



class puttyField: public MPxFieldNode
{
public:
	puttyField() {};
	virtual ~puttyField() {};

	static void		*creator();
	static MStatus	initialize();

	// will compute output force.
	//
	virtual MStatus	compute( const MPlug& plug, MDataBlock& block );
	virtual MStatus	computeScriptSourced( const MPlug& plug, MDataBlock& block );
	virtual MStatus	computeNodeReady( const MPlug& plug, MDataBlock& block );

	// attributes.
	//
    static  MObject aScript;            // name of script to use
	static  MObject aCmdBaseName;		// name of the command
    static  MObject aSource;            // flag indicating a resourcing of the script
	static  MObject aNodeReady;         // flag indicating the node is ready for computation
	static  MObject aScriptSourced;     // flag indicating the script is sourced
//  static  MObject aDynDirty;          // helper flag used to pass on dirty information for dynamic attributes

    // current values - they are a storage position for values that can be used by the script
    static  MObject aCurrPosition;         // the current position
    static  MObject aCurrVelocity;         // the current velocity
    static  MObject aCurrMass;             // the current mass
    static  MObject aCurrDeltaTime;        // current delta time in seconds
    static  MObject aCurrFieldPositionCount;       // the nr of elements in the field position array
    static  MObject aCurrFieldPosition;    // the position of the field owner
    static  MObject aCurrMultiIndex; 
     

	// Other data members
	//
	static MTypeId	id;

private:

	void	ownerPosition( MDataBlock& block, MVectorArray &vArray );
	MStatus	getWorldPosition( MVector &vector );
	MStatus	getWorldPosition( MDataBlock& block, MVector &vector );

	// methods to get attribute value.

	bool	applyPerVertexValue( MDataBlock& block );
	MStatus	ownerCentroidValue( MDataBlock& block, MVector &vector );
};



inline bool puttyField::applyPerVertexValue( MDataBlock& block )
{
	MStatus status;

	MDataHandle hValue = block.inputValue( mApplyPerVertex, &status );

	bool value = false;
	if( status == MS::kSuccess )
		value = hValue.asBool();

	return( value );
}



inline MStatus puttyField::ownerCentroidValue(MDataBlock& block,MVector &vector)
{
	MStatus status;

	MDataHandle hValueX = block.inputValue( mOwnerCentroidX, &status );
	MDataHandle hValueY = block.inputValue( mOwnerCentroidY, &status );
	MDataHandle hValueZ = block.inputValue( mOwnerCentroidZ, &status );

	if( status == MS::kSuccess )
	{
		vector[0] = hValueX.asDouble();
		vector[1] = hValueY.asDouble();
		vector[2] = hValueZ.asDouble();
	}

	return( status );
}




#endif
