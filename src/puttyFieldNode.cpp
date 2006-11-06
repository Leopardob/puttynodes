// Title: puttyField
//
// About:
// This field node let's you script the function generating a field force yourself using a mel script.

#include <maya/MIOStream.h>
#include <math.h>

#include "../include/helperMacros.h"
#include "../include/puttyFieldNode.h"
#include "../include/puttyNodeIds.h"


#include <maya/MVectorArray.h>
#include <maya/MDoubleArray.h>

#include <maya/MFnDependencyNode.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnVectorArrayData.h>
#include <maya/MFnDoubleArrayData.h>
#include <maya/MFnMatrixData.h>
#include <maya/MMatrix.h>
#include <maya/MCommandResult.h>
#include <maya/MGlobal.h>





MObject puttyField::aScript;   // the script to source
MObject puttyField::aCmdBaseName;  // the base name of the command to execute
MObject puttyField::aSource;           
MObject puttyField::aNodeReady; 
MObject puttyField::aScriptSourced;


MObject puttyField::aCurrPosition;     // the current position
MObject puttyField::aCurrVelocity;     // the current velocity
MObject puttyField::aCurrMass;         // the current mass
MObject puttyField::aCurrDeltaTime;    // current delta time in seconds
MObject puttyField::aCurrFieldPosition;      // the current owner position
MObject puttyField::aCurrFieldPositionCount;      // the current owner position
MObject puttyField::aCurrMultiIndex; 

MTypeId puttyField::id( PUTTYFIELD_NODE_ID );





void *puttyField::creator()
{
    return new puttyField;
}


MStatus puttyField::initialize()
//
//	Descriptions:
//		Initialize the node, attributes.
//
{
	MStatus status;

	MFnNumericAttribute		nAttr;
    MFnTypedAttribute       tAttr;

	// script management
	//
	aScript = tAttr.create( "script", "scr", MFnData::kString);
    tAttr.setStorable(true);
    tAttr.setKeyable(false);
	SYS_ERROR_CHECK( addAttribute( aScript ), "adding aScript" );  

	aCmdBaseName = tAttr.create( "cmdBaseName", "cbn", MFnData::kString);
    tAttr.setStorable(true);
    tAttr.setKeyable(false);
    tAttr.setHidden(true);    
	SYS_ERROR_CHECK( addAttribute( aCmdBaseName ), "adding aCmdBaseName" );  

    // refresh
    
    aSource = nAttr.create( "source", "src", MFnNumericData::kBoolean, 0 );
    nAttr.setHidden(true);    
    SYS_ERROR_CHECK( addAttribute( aSource ), "adding aSource" ); 
    
	// it is important that script sourced is initialised false and not storable
    // so this way the function gets sourced on maya startup    
    aScriptSourced = nAttr.create( "scriptSourced", "ssrc", MFnNumericData::kBoolean, 0 );
    nAttr.setStorable(false);
    nAttr.setHidden(true);        
    SYS_ERROR_CHECK( addAttribute( aScriptSourced ), "adding aScriptSourced" ); 


    aNodeReady = nAttr.create( "nodeReady", "nr", MFnNumericData::kBoolean, 0 );
    nAttr.setHidden(true);    
    SYS_ERROR_CHECK( addAttribute( aNodeReady ), "adding aNodeReady" );  
 
    /////////////////////////////////////////////////////////////////////////////    
    // current values
    aCurrPosition = tAttr.create( "currentPosition", "cpos", MFnData::kVectorArray);
    tAttr.setStorable(false);
    tAttr.setKeyable(false);
    tAttr.setConnectable(false);
    tAttr.setWritable(false);
	SYS_ERROR_CHECK( addAttribute( aCurrPosition ), "adding aCurrPosition" );          

    aCurrVelocity = tAttr.create( "currentVelocity", "cvel", MFnData::kVectorArray);
    tAttr.setStorable(false);
    tAttr.setKeyable(false);
    tAttr.setConnectable(false);
    tAttr.setWritable(false);
	SYS_ERROR_CHECK( addAttribute( aCurrVelocity ), "adding aCurrVelocity" );          

    aCurrMass = tAttr.create( "currentMass", "cmas", MFnData::kDoubleArray);
    tAttr.setStorable(false);
    tAttr.setKeyable(false);
    tAttr.setConnectable(false);
    tAttr.setWritable(false);
	SYS_ERROR_CHECK( addAttribute( aCurrMass ), "adding aCurrMass" );          
    
    aCurrDeltaTime = nAttr.create("aCurrDeltaTime","cdt",MFnNumericData::kDouble);
    nAttr.setStorable(false);
    nAttr.setKeyable(false);
    nAttr.setConnectable(false);
    nAttr.setWritable(false);
	SYS_ERROR_CHECK( addAttribute( aCurrDeltaTime ), "adding aCurrDeltaTime" );          

    aCurrFieldPositionCount = nAttr.create("currentFieldPositionCount","cfpc",MFnNumericData::kInt);
    nAttr.setStorable(false);
    nAttr.setKeyable(false);
    nAttr.setConnectable(false);
    nAttr.setWritable(false);
	SYS_ERROR_CHECK( addAttribute( aCurrFieldPositionCount ), "adding aCurrFieldCount" );          

    aCurrFieldPosition = tAttr.create( "currentFieldPosition", "cfpos", MFnData::kVectorArray);
    tAttr.setStorable(false);
    tAttr.setKeyable(false);
    tAttr.setConnectable(false);
    tAttr.setWritable(false);
	SYS_ERROR_CHECK( addAttribute( aCurrFieldPosition ), "adding aCurrFieldPosition" );
    

    aCurrMultiIndex = nAttr.create("currentMultiIndex","cmi",MFnNumericData::kInt);
    nAttr.setStorable(false);
    nAttr.setKeyable(false);
    nAttr.setConnectable(false);
    nAttr.setWritable(false);
	SYS_ERROR_CHECK( addAttribute( aCurrMultiIndex ), "adding aCurrMultiIndex" );  


    /////////////////////////////////////////////////////////////////////////////    
    // affects
    attributeAffects( aScript ,aNodeReady);          
    attributeAffects( aSource, aNodeReady  );
    attributeAffects( aScriptSourced, aNodeReady  );
    
	attributeAffects( aScript,aScriptSourced);
	attributeAffects( aSource,aScriptSourced);


	return( MS::kSuccess );
}


//
//		compute output force.
//
MStatus	puttyField::computeScriptSourced( const MPlug& plug, MDataBlock& block )
{
	MStatus status;
  
  
        // this part of the function sources the script
    	// try to source the script

        bool result = true;
        
        // get the source flag
		MDataHandle dh = block.inputValue(aSource,&status);
		SYS_ERROR_CHECK(status, "Error getting source data handle\n");
		bool source = dh.asBool();
        
        // get the script
		dh = block.inputValue(aScript,&status);
		SYS_ERROR_CHECK(status, "Error getting reload script handle\n");    
		MString script =  dh.asString();         
        
		MString cmd = "source \"" + script+"\"";
	    MCommandResult melResult;
		status = MGlobal::executeCommand(cmd,melResult);
		
        if (status.error())
		{
			MGlobal::displayError( "Error sourcing mel script, please check the function you provided is valid!");
            result = false;
		}

        // set the result        
		MDataHandle dhScriptSourced = block.outputValue(aScriptSourced,&status);
		SYS_ERROR_CHECK(status, "Error getting ScriptSourced data handle\n");
		dhScriptSourced.set(result);
		dhScriptSourced.setClean();        
        
        return MS::kSuccess;


}

MStatus	puttyField::computeNodeReady( const MPlug& plug, MDataBlock& block )
{
	MStatus status;
	bool result =false;

		MString cmdBaseName;

		// get the source flag
		MDataHandle dh = block.inputValue(aSource,&status);
		SYS_ERROR_CHECK(status, "Error getting source data handle\n");
		bool source = dh.asBool();
    
		// get the command
		dh = block.inputValue(aScript,&status);
		SYS_ERROR_CHECK(status, "Error getting reload script handle\n");    
		MString script =  dh.asString(); 

		if (script == "")
		{
			MGlobal::displayError("no script provided!\n");
		}
		else
		{
            // chech if script is sourced
        	dh = block.inputValue(aScriptSourced,&status);
        	SYS_ERROR_CHECK(status, "Error getting aScriptSourced data handle\n");
        	bool scriptSourced = dh.asBool();

        	// if it's not ready, don't do anything
        	if (!scriptSourced)
        		return MS::kSuccess;
			else
			{
       		    MCommandResult melResult;

				// now get the real name of the function and store it in a separate attribute
				MString cmd="basenameEx \"" + script+"\"";
				status = MGlobal::executeCommand(cmd,melResult);
				melResult.getResult(cmdBaseName);
				result = true;
				
				MDataHandle dhCBN = block.outputValue(aCmdBaseName,&status);
				SYS_ERROR_CHECK(status, "Error getting aCmdBaseName data handle\n");
				dhCBN.set(cmdBaseName);
				dhCBN.setClean();

				// see if an interface function is present, if yes, execute it
				cmd= "if(exists(\"" + cmdBaseName +".interface\")) {";
				cmd+= "string $attr[] = `deleteAttr -q " +name()+"`; string $a;";
				cmd+="for($a in $attr) deleteAttr (\""+name()+".\"+$a);";
				cmd+= cmdBaseName +".interface(\"" +name()+"\");}";
				status = MGlobal::executeCommand(cmd);
			}

		}

		// check the current status
		
		// set the result
		MDataHandle dhNodeReady = block.outputValue(aNodeReady,&status);
		SYS_ERROR_CHECK(status, "Error getting reload data handle\n");
		dhNodeReady.set(result);
		dhNodeReady.setClean();

		return MS::kSuccess;
}


MStatus puttyField::compute(const MPlug& plug, MDataBlock& block)
{
	MStatus status;

    //only compute if we want the output  
	if ( plug == aNodeReady )
		return computeNodeReady(plug,block);
	else if (plug == aScriptSourced)
		return computeScriptSourced(plug,block);
	else if( !(plug == mOutputForce) )
        return( MS::kUnknownParameter );

	
   /////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // get inputs
    //
	
	// get the node ready flag
	MDataHandle dh = block.inputValue(aScriptSourced,&status);
	SYS_ERROR_CHECK(status, "Error getting aScriptSourced data handle\n");
	bool scriptSourced = dh.asBool();
	if (!scriptSourced)
		return MS::kSuccess;


	dh = block.inputValue(aNodeReady,&status);
	SYS_ERROR_CHECK(status, "Error getting node ready data handle\n");
	bool nodeReady = dh.asBool();

	// if it's not ready, don't do anything
	if (!nodeReady)
		return MS::kSuccess;

    // get the command
    dh = block.inputValue(aCmdBaseName,&status);
    SYS_ERROR_CHECK(status, "Error getting aCmdBaseName  handle\n");    
    MString script =  dh.asString(); 

    
    // get the logical index of the element this plug refers to.
	//
	int multiIndex = plug.logicalIndex( &status );
	SYS_ERROR_CHECK(status, "ERROR in plug.logicalIndex.\n");

	// Get input data handle, use outputArrayValue since we do not
	// want to evaluate both inputs, only the one related to the
	// requested multiIndex. Evaluating both inputs at once would cause
	// a dependency graph loop.
	//
	MArrayDataHandle hInputArray = block.outputArrayValue( mInputData, &status );
	SYS_ERROR_CHECK(status,"ERROR in hInputArray = block.outputArrayValue().\n");

	status = hInputArray.jumpToElement( multiIndex );
	SYS_ERROR_CHECK(status, "ERROR: hInputArray.jumpToElement failed.\n");

	// get children of aInputData.
	//
	MDataHandle hCompond = hInputArray.inputValue( &status );
	SYS_ERROR_CHECK(status, "ERROR in hCompond=hInputArray.inputValue\n");

	MDataHandle hPosition = hCompond.child( mInputPositions );
	MObject dPosition = hPosition.data();
	MFnVectorArrayData fnPosition( dPosition );
	MVectorArray points = fnPosition.array( &status );
	SYS_ERROR_CHECK(status, "ERROR in fnPosition.array(), not find points.\n");

	MDataHandle hVelocity = hCompond.child( mInputVelocities );
	MObject dVelocity = hVelocity.data();
	MFnVectorArrayData fnVelocity( dVelocity );
	MVectorArray velocities = fnVelocity.array( &status );
	SYS_ERROR_CHECK(status, "ERROR in fnVelocity.array(), not find velocities.\n");

	MDataHandle hMass = hCompond.child( mInputMass );
	MObject dMass = hMass.data();
	MFnDoubleArrayData fnMass( dMass );
	MDoubleArray masses = fnMass.array( &status );
	SYS_ERROR_CHECK(status, "ERROR in fnMass.array(), not find masses.\n");
    

	MDataHandle hDt = hCompond.child( mDeltaTime );
    double deltaTime = hDt.asDouble();
    
    // count
    int count = points.length();
    
    // get the owner field positions
    MVectorArray fieldPosition;	fieldPosition.clear();
	ownerPosition( block, fieldPosition );

	int fieldPosCount = fieldPosition.length();

	///////////////////////////////////////////////////////////////////////////////////////
    // Compute the output force.
	//
	
    // set the current inputs on the interface plugs
    
     // the position
    MObject thisNode = thisMObject();
    
    MPlug currPlug(thisNode,aCurrPosition);
    MFnVectorArrayData vecD;
    MObject currObj = vecD.create(points,&status);
    currPlug.setValue(currObj);
    SYS_ERROR_CHECK(status, "error setting currPosPlug value\n");
    
    // velocity
    currPlug = MPlug(thisNode,aCurrVelocity);
    currObj = vecD.create(velocities,&status);
    currPlug.setValue(currObj);
    SYS_ERROR_CHECK(status, "error setting currPosPlug value\n");

    // the weights
    currPlug =MPlug(thisNode,aCurrMass);
    MFnDoubleArrayData dblD;
    currObj = dblD.create(masses,&status);
    currPlug.setValue(currObj);
    SYS_ERROR_CHECK(status, "error setting currMassPlug value\n");
    
    // field positions
    currPlug= MPlug(thisNode,aCurrFieldPosition);
    currObj = vecD.create(fieldPosition,&status);
    currPlug.setValue(currObj);
    SYS_ERROR_CHECK(status, "error setting fieldPositionPlug value\n");
    
    
    // the delta time
    currPlug =MPlug(thisNode,aCurrDeltaTime);
    currPlug.setValue(deltaTime);
    SYS_ERROR_CHECK(status, "error setting aCurrDeltaTime value\n");


    // the multi index
    currPlug =MPlug(thisNode,aCurrMultiIndex);
    currPlug.setValue(int(multiIndex));
    SYS_ERROR_CHECK(status, "error setting currMultiIndexPlug value\n");
    
    // the field pos count
    currPlug =MPlug(thisNode,aCurrFieldPositionCount);
    currPlug.setValue(fieldPosCount);
    SYS_ERROR_CHECK(status, "error setting aCurrFieldPosCount value\n");


   
    /////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // execute the mel script
    //
    MString melCmd = script+"(\"" +name()+"\","+count+")";
    
    MCommandResult melResult;
    status = MGlobal::executeCommand(melCmd,melResult);
    USER_ERROR_CHECK(status, "Error executing mel command, please check the function you provided is valid and has the appropriate parameters!");

    // check the result type
    if ((melResult.resultType()) != (MCommandResult::kDoubleArray))
    {
        USER_ERROR_CHECK(MS::kFailure, "result of mel command has wrong type, should be doubleArray (which will be interpreted as vectorArray)!");
    }
    
    // get the result as a double array
    MDoubleArray newF;  
    status = melResult.getResult(newF);
    USER_ERROR_CHECK(status, "Error getting result of mel command!");
    
    int newCount = newF.length()/3;
    // size check
    if (newCount != count)
    {
        USER_ERROR_CHECK(MS::kFailure, "the size of the result does not match the size of the input!");
    }

    // convert the double array into a vector array
     MVectorArray outForce(newCount);
    
    for(int i=0;i<newCount;i++)
        outForce[i]=MVector(newF[i*3],newF[i*3+1],newF[i*3+2]);
           
    
    
    
    /////////////////////////////////////////////////////////////////////////////////////////
    // output the data
	//
    
	MArrayDataHandle hOutArray = block.outputArrayValue( mOutputForce, &status);
	SYS_ERROR_CHECK(status, "ERROR in hOutArray = block.outputArrayValue.\n");
	MArrayDataBuilder bOutArray = hOutArray.builder( &status );
	SYS_ERROR_CHECK(status, "ERROR in bOutArray = hOutArray.builder.\n");

	// get output force array from block.
	//
	MDataHandle hOut = bOutArray.addElement(multiIndex, &status);
	SYS_ERROR_CHECK(status, "ERROR in hOut = bOutArray.addElement.\n");

	MFnVectorArrayData fnOutputForce;
	MObject dOutputForce = fnOutputForce.create( outForce, &status );
	SYS_ERROR_CHECK(status, "ERROR in dOutputForce = fnOutputForce.create\n");

	// update data block with new output force data.
	//
	hOut.set( dOutputForce );
	block.setClean( plug );

	return( MS::kSuccess );
}





void puttyField::ownerPosition
	(
		MDataBlock& block,
		MVectorArray &ownerPosArray
	)
//
//	Descriptions:
//		If this field has an owner, get the owner's position array or
//		centroid, then assign it to the ownerPosArray.
//		If it does not have owner, get the field position in the world
//		space, and assign it to the given array, ownerPosArray.
//
{
	MStatus status;

	if( applyPerVertexValue(block) )
	{
		MDataHandle hOwnerPos = block.inputValue( mOwnerPosData, &status );
		if( status == MS::kSuccess )
		{
			MObject dOwnerPos = hOwnerPos.data();
			MFnVectorArrayData fnOwnerPos( dOwnerPos );
			MVectorArray posArray = fnOwnerPos.array( &status );
			if( status == MS::kSuccess )
			{
				// assign vectors from block to ownerPosArray.
				//
				for( unsigned int i = 0; i < posArray.length(); i ++ )
					ownerPosArray.append( posArray[i] );
			}
			else
			{
				MVector worldPos(0.0, 0.0, 0.0);
				//status = getWorldPosition( block, worldPos );
				status = getWorldPosition( worldPos );
				ownerPosArray.append( worldPos );
			}
		}
		else
		{
			// get the field position in the world space
			// and add it into ownerPosArray.
			//
			MVector worldPos(0.0, 0.0, 0.0);
			//status = getWorldPosition( block, worldPos );
			status = getWorldPosition( worldPos );
			ownerPosArray.append( worldPos );
		}
	}
	else
	{
		MVector centroidV(0.0, 0.0, 0.0);
		status = ownerCentroidValue( block, centroidV );
		if( status == MS::kSuccess )
		{
			// assign centroid vector to ownerPosArray.
			//
			ownerPosArray.append( centroidV );
		}
		else
		{
			// get the field position in the world space.
			//
			MVector worldPos(0.0, 0.0, 0.0);
			//status = getWorldPosition( block, worldPos );
			status = getWorldPosition( worldPos );
			ownerPosArray.append( worldPos );
		}
	}
}


MStatus puttyField::getWorldPosition( MVector &vector )
//
//	Descriptions:
//		get the field position in the world space.
//		The position value is from inherited attribute, aWorldMatrix.
//
{
	MStatus status;

	MObject thisNode = thisMObject();
	MFnDependencyNode fnThisNode( thisNode );

	// get worldMatrix attribute.
	//
	MObject worldMatrixAttr = fnThisNode.attribute( "worldMatrix" );

	// build worldMatrix plug, and specify which element the plug refers to.
	// We use the first element(the first dagPath of this field).
	//
	MPlug matrixPlug( thisNode, worldMatrixAttr );
	matrixPlug = matrixPlug.elementByLogicalIndex( 0 );

	// Get the value of the 'worldMatrix' attribute
	//
	MObject matrixObject;
	status = matrixPlug.getValue( matrixObject );
	if( !status )
	{
		status.perror("puttyField::getWorldPosition: get matrixObject");
		return( status );
	}

	MFnMatrixData worldMatrixData( matrixObject, &status );
	if( !status )
	{
		status.perror("puttyField::getWorldPosition: get worldMatrixData");
		return( status );
	}

	MMatrix worldMatrix = worldMatrixData.matrix( &status );
	if( !status )
	{
		status.perror("puttyField::getWorldPosition: get worldMatrix");
		return( status );
	}

	// assign the translate to the given vector.
	//
	vector[0] = worldMatrix( 3, 0 );
	vector[1] = worldMatrix( 3, 1 );
	vector[2] = worldMatrix( 3, 2 );

    return( status );
}


MStatus puttyField::getWorldPosition( MDataBlock& block, MVector &vector )
//
//	Descriptions:
//		Find the field position in the world space.
//
{
    MStatus status;

	MObject thisNode = thisMObject();
	MFnDependencyNode fnThisNode( thisNode );

	// get worldMatrix attribute.
	//
	MObject worldMatrixAttr = fnThisNode.attribute( "worldMatrix" );

	// build worldMatrix plug, and specify which element the plug refers to.
	// We use the first element(the first dagPath of this field).
	//
	MPlug matrixPlug( thisNode, worldMatrixAttr );
	matrixPlug = matrixPlug.elementByLogicalIndex( 0 );

    //MDataHandle hWMatrix = block.inputValue( worldMatrix, &status );
    MDataHandle hWMatrix = block.inputValue( matrixPlug, &status );

	SYS_ERROR_CHECK(status, "ERROR getting hWMatrix from dataBlock.\n");

    if( status == MS::kSuccess )
    {
        MMatrix wMatrix = hWMatrix.asMatrix();
        vector[0] = wMatrix(3, 0);
        vector[1] = wMatrix(3, 1);
        vector[2] = wMatrix(3, 2);
    }
    return( status );
}



