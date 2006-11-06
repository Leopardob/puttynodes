// Title: puttyDeformer
//
// About:
// This deformer node let's you script the deformation function yourself using a mel script. While this is not as fast
// as a pure C++ implementation, it gives you great flexibility and if nothing else - a good testing ground before 
// implementing your deformer using the Maya API directly. The speed issues might not be a problem if you are just looking for a
// "one-off" solution.
//
// Creation:
// To create an puttyDeformer select a piece of geometry and use "Create puttyDeformer" from the melfunctions menu.
// Alternativly type "puttyDeformer" into the command window. Like any other deformer it could also be created using the
// "deformer -type puttyDeformer" mel command, though this won't add support for paintable weights.
//
// Basic Usage:
// Whenever the output of the deformer is requested, a mel script is called which generated new positions, these are the positions 
// object components will have after the deformation.
//
// The mel script provided has to be available to the node (it must be a global funtion previously sourced).
// 
// The script must have the following paramaters:
//
// - Parameter 1: string $deformerName
// your function always has to have a parameter for the name of the deformer which is calling it, you can use it to retrieve additional
// information about the the currently deformed geometry
// - Parameter 2: int $count
// your function always has to have a parameter for the number of components that are being deformed,
// this number helps you to construct the output positions of the deformer
// *Important:* This number relates to the number of components, not the length of the array that you have to return. As each component
// has 3 positional parameter (X,Y,Z) the final doubleArray you have to return has to be of length 3*$count!
// 
// Now inside the script you can query more informations about the currently deformed geometry, you do this using the 'getAttr' command.
// Here is the list of available values
//
// - *currentPosition*, these are all the component positions of the geometry before the deformation
// - *envelope*, how much effect does the deformer have on the geometry
// - *currentWeight*, the individual weights per component position (can be painted)
// - *currentWorldMatrix*, the world matrix of the geometry, can be used to transform the positions into world space
// - *currentMultiIndex*, deformers can deform more than one geometry, this value gives the id of the current one
// - *currentGeometryName*, name of the geometry (will be the long name!)
// - *currentGeometryType*, type of the geometry (mesh/nurbs/nurbsCurve/particle)
//
// You might wonder why all those values have to be accessed manually while the obvious might have been to pass them to the
// function as parameters? This is simply because of performance - building the parameter string within Maya's API is veeery slow,
// and accessing the data directly speeds up the execution a lot!
//
// Painting weights:
// TODO
//
// Adding additional variables / Accesing data:
// The values provided are very likely not sufficient to control the scripted deformation. To conviniently control variables in
// your script, add dynamic attributes to the deformer node and access them using the same 'getAttr' command. If the dynamic
// attributes change, they will automaticly trigger an evaluation of the deformer (but only if they were added to the deformer!)
// So if you want to access other data within your scene, eg. a locators translate value, it would be best to first add a
// dynamic vector attribute to the deformer and the connect this one with the locators translate. This way your attribute acts as
// an interface to the outside scene world and the deformer can update correctly.
// 
// Examples:
// Can be found in the /melfunctions/mel/puttyDeformer_examples/ directory and should be self explenatory!


// maya includes
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnUnitAttribute.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MFnMatrixAttribute.h>
#include <maya/MFnVectorArrayData.h>
#include <maya/MFnDoubleArrayData.h>
#include <maya/MFnMatrixData.h>

#include <maya/MItMeshVertex.h>

#include <maya/MTime.h>
#include <maya/MPoint.h>
#include <maya/MPointArray.h>
#include <maya/MVector.h>
#include <maya/MMatrix.h>
#include <maya/MCommandResult.h>
#include <maya/MPlugArray.h>



#include "../include/helperMacros.h"

#include "../include/puttyNodeIds.h"
#include "../include/puttyDeformerNode.h"




/*************************************************************/

MTypeId puttyDeformer::id( PUTTYDEFORMER_NODE_ID );

     
MObject puttyDeformer::aScript;   // the script to source
MObject puttyDeformer::aCmdBaseName;  // the base name of the command to execute
MObject puttyDeformer::aSource;           
MObject puttyDeformer::aNodeReady; 
MObject puttyDeformer::aScriptSourced;
MObject puttyDeformer::aDynDirty; 


MObject puttyDeformer::aDefSpace;         // which space is the deformer working in
MObject puttyDeformer::aDefWeights;       // deformer weights
MObject puttyDeformer::aDefEnvelope;      // envelope

MObject puttyDeformer::aCurrPosition;     // the current position         
MObject puttyDeformer::aCurrWorldMatrix;
MObject puttyDeformer::aCurrMultiIndex;   // the current multi index
MObject puttyDeformer::aCurrWeight;       // the current weight        
//MObject puttyDeformer::aCurrGeometryName;   // current object name
MObject puttyDeformer::aCurrGeometryType; // current object 


puttyDeformer:: puttyDeformer() {}
puttyDeformer::~puttyDeformer() {}

/*************************************************************************************/
void* puttyDeformer::creator() 
{ 
	return new puttyDeformer(); 
}

/*************************************************************************************/
// function to mark the output dirty once one of the dynamic attributes changes
MStatus puttyDeformer::setDependentsDirty( const MPlug &plugBeingDirtied,	MPlugArray &affectedPlugs )
{
	if ( plugBeingDirtied.isDynamic()) 
    {
		MStatus	status;
		MObject thisNode = thisMObject();

		// there is a dynamic attribute that is is dirty, so mark 
        // the outputGeometry dirty

		if ( MStatus::kSuccess == status ) 
		{
			cerr << "\n###def dirty " << plugBeingDirtied.name() ;

			MPlug pB( thisNode, puttyDeformer::outputGeom );
	         affectedPlugs.append( pB );

//			MPlug pD( thisNode, puttyDeformer::aDynDirty );
//			pD.setValue(true);
    
//			affectedPlugs.append( output );

		}

	}
	return( MS::kSuccess );
}

/*************************************************************************************/
MStatus puttyDeformer::initialize()
{
	MStatus status;
	MFnNumericAttribute		nAttr;
  	MFnEnumAttribute		eAttr;
    MFnTypedAttribute       tAttr;
    MFnMatrixAttribute       mAttr;

    // the script


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

    aDynDirty = nAttr.create( "dynDirty", "dd", MFnNumericData::kBoolean, 0 );
    nAttr.setHidden(true);    
    SYS_ERROR_CHECK( addAttribute( aDynDirty ), "adding aDynDirty" ); 

  
    // space
   	aDefSpace = eAttr.create("deformerSpace", "dsp", MSD_SPACE_OBJECT, &status);
	eAttr.addField("object (default)", MSD_SPACE_OBJECT);
	eAttr.addField("world (automatic conversion)", MSD_SPACE_WORLD);
   	eAttr.setKeyable(false);
	eAttr.setStorable(true);
	SYS_ERROR_CHECK( addAttribute( aDefSpace ), "adding aDefSpace" );

    // envelope
   	aDefEnvelope = eAttr.create("deformerEnvelope", "de", MSD_ENVELOPE_AUTO, &status);
	eAttr.addField("auto", MSD_ENVELOPE_AUTO);
    eAttr.addField("user", MSD_ENVELOPE_USER);    
   	eAttr.setKeyable(false);
	eAttr.setStorable(true);
	SYS_ERROR_CHECK( addAttribute( aDefEnvelope ), "adding aDefEnvelope" );

    // weights
   	aDefWeights = eAttr.create("deformerWeights", "dw", MSD_WEIGHTS_AUTO, &status);
	eAttr.addField("auto", MSD_WEIGHTS_AUTO);
    eAttr.addField("user", MSD_WEIGHTS_USER);    
   	eAttr.setKeyable(false);
	eAttr.setStorable(true);
	SYS_ERROR_CHECK( addAttribute( aDefWeights ), "adding aDefWeights" );

    /////////////////////////////////////////////////////////////////////////////    
    // current values
    aCurrPosition = tAttr.create( "currentPosition", "cpos", MFnData::kVectorArray);
    tAttr.setStorable(false);
    tAttr.setKeyable(false);
    tAttr.setConnectable(false);
    tAttr.setWritable(false);
	SYS_ERROR_CHECK( addAttribute( aCurrPosition ), "adding aCurrPos" );          

    aCurrWeight = tAttr.create( "currentWeight", "cwgh", MFnData::kDoubleArray);
    tAttr.setStorable(false);
    tAttr.setKeyable(false);
    tAttr.setConnectable(false);
    tAttr.setWritable(false);
	SYS_ERROR_CHECK( addAttribute( aCurrWeight ), "adding aCurrWeight" );          
    
    aCurrMultiIndex = nAttr.create("currentMultiIndex","cmi",MFnNumericData::kInt);
    nAttr.setStorable(false);
    nAttr.setKeyable(false);
    nAttr.setConnectable(false);
    nAttr.setWritable(false);
	SYS_ERROR_CHECK( addAttribute( aCurrMultiIndex ), "adding aCurrMultiIndex" );          

    aCurrWorldMatrix = mAttr.create("currentWorldMatrix","cwm"); 
    mAttr.setStorable(false);
    mAttr.setKeyable(false);
    mAttr.setConnectable(false);
    mAttr.setWritable(false);
	SYS_ERROR_CHECK( addAttribute( aCurrWorldMatrix ), "adding aCurrObjectName" );          

/*
    aCurrGeometryName= tAttr.create( "currentGeometryName", "con", MFnData::kString);
    tAttr.setStorable(false);
    tAttr.setKeyable(false);
    tAttr.setConnectable(false);
    tAttr.setWritable(false);
	SYS_ERROR_CHECK( addAttribute( aCurrGeometryName ), "adding aCurrObjectName" );          
*/       
    aCurrGeometryType= tAttr.create( "currentGeometryType", "cot", MFnData::kString);
    tAttr.setStorable(false);
    tAttr.setKeyable(false);
    tAttr.setConnectable(false);
    tAttr.setWritable(false);
	SYS_ERROR_CHECK( addAttribute( aCurrGeometryType ), "adding aCurrGeometryType" );


    /////////////////////////////////////////////////////////////////////////////    
    // affects
    attributeAffects( aScript ,aNodeReady);          
    attributeAffects( aSource, aNodeReady  );
    attributeAffects( aScriptSourced, aNodeReady  );
    
	attributeAffects( aScript,aScriptSourced);
	attributeAffects( aSource,aScriptSourced);
	
    attributeAffects( aScriptSourced, outputGeom  );    
	attributeAffects( aDynDirty,outputGeom ); 
	attributeAffects( aNodeReady,outputGeom ); 
    attributeAffects( aScript ,outputGeom); 
	attributeAffects(  aCmdBaseName  ,outputGeom);   
    attributeAffects( aSource ,outputGeom );        
    attributeAffects( aDefSpace, outputGeom  );
    attributeAffects( aDefWeights,outputGeom);
    attributeAffects( aDefEnvelope,outputGeom ); 
	return MS::kSuccess;
}

/*************************************************************************************/
MStatus	puttyDeformer::compute( const MPlug& plug, MDataBlock& block )
{
	MStatus status;
	if ( plug == aNodeReady )
	{
//		MGlobal::displayInfo("compute");
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
    else if (plug==aScriptSourced)
    {
        // this part of the function sources the script
    	// try to source the script
//      cerr << "\nsource";
        
        MStatus status;
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
	return MS::kUnknownParameter;
}

/*************************************************************************************/
MStatus puttyDeformer::deform( MDataBlock& block, MItGeometry& iter, const MMatrix& worldMatrix, unsigned int multiIndex)
{
//	cerr<<"\ndeform";

    MStatus status = MS::kSuccess;

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

    dh = block.inputValue(aDefSpace,&status);
    SYS_ERROR_CHECK(status, "Error getting defSpace data handle\n");
    short defSpace = dh.asShort();
    
    dh = block.inputValue(aDefWeights,&status);
    SYS_ERROR_CHECK(status, "Error getting defWeights data handle\n");
    short defWeights = dh.asShort();
 
    dh = block.inputValue(aDefEnvelope,&status);
    SYS_ERROR_CHECK(status, "Error getting defEnvelope data handle\n");
    short defEnvelope = dh.asShort();
    

    
    // get the command
    dh = block.inputValue(aCmdBaseName,&status);
    SYS_ERROR_CHECK(status, "Error getting aCmdBaseName  handle\n");    
    MString script =  dh.asString(); 
        
 /*   if (script == "")
    {
        status = MS::kFailure;
        USER_ERROR_CHECK(status, "no script provided!\n");    
    }
   */ 
    /////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // build mel cmd string
    //
    
    // check if it's a valid cmd
        
   
    // get the envelope
    //
    double env = 1;
    
    if (defEnvelope == MSD_ENVELOPE_AUTO)
    {
        dh = block.inputValue(envelope,&status);
    	SYS_ERROR_CHECK(status, "Error getting envelope data handle\n");	
	    env = double(dh.asFloat());	
        
        // early stop 'cause there is nothing more to do
        if (env == 0.0)
            return MS::kSuccess;
    }
    
    // get the points, transform them into the right space if needed
    //
    int count = iter.count();
    MVectorArray points(count);
    for ( ; !iter.isDone(); iter.next()) 
        points[iter.index()] = iter.position();
        
    if ( defSpace == MSD_SPACE_WORLD )
    {
        for (int i = 0;i<count;i++)
            points[i] = MPoint(points[i]) * worldMatrix;
    }
    
    
    // get the weights
    //
    MDoubleArray weights;
    if ( defWeights == MSD_WEIGHTS_AUTO)
    {
        weights.setLength(count);
        
        for (int i = 0;i<count;i++)
            weights[i]  = weightValue(block,multiIndex,i);
        
    }


    // get the object name and type
    // get the input geometry, traverse through the data handles    
    MArrayDataHandle adh = block.outputArrayValue( input, &status );
    SYS_ERROR_CHECK(status,"error getting input array data handle.\n");

    status = adh.jumpToElement( multiIndex );
    SYS_ERROR_CHECK(status, "input jumpToElement failed.\n");

    // compound data 
    MDataHandle cdh = adh.inputValue( &status );
    SYS_ERROR_CHECK(status, "error getting input inputValue\n");
   
    // input geometry child
    dh = cdh.child( inputGeom );
    MObject dInputGeometry = dh.data();
   
    // get the type      
    MString geometryType = dInputGeometry.apiTypeStr();

    // get the name    
//    MFnDagNode dagFn( dInputGeometry, &status);
//    SYS_ERROR_CHECK(status, "error converting geometry obj to dag node\n");
   
//    MString geometryName = dagFn.fullPathName(&status);
//    SYS_ERROR_CHECK(status, "error getting full path name \n");

//    MString geometryType = "";
//    MString geometryName = "";
    
    /////////////////////////////////////////////////////////////////////////////////////////////////
    //  
    //  set the current values on the temp plugs for the script to be picked up
    //
    
    // the position
    MObject thisNode = thisMObject();
    
    MPlug currPlug(thisNode,aCurrPosition);
    MFnVectorArrayData vecD;
    MObject currObj = vecD.create(points,&status);
    currPlug.setValue(currObj);
    SYS_ERROR_CHECK(status, "error setting currPosPlug value\n");
    
    // the weights
    currPlug =MPlug(thisNode,aCurrWeight);
    MFnDoubleArrayData dblD;
    currObj = dblD.create(weights,&status);
    currPlug.setValue(currObj);
    SYS_ERROR_CHECK(status, "error setting currWeightsPlug value\n");
    
    // world matrix
    currPlug =MPlug(thisNode,aCurrWorldMatrix);
    MFnMatrixData matD;
    currObj = matD.create(worldMatrix,&status);
    currPlug.setValue(currObj);
    SYS_ERROR_CHECK(status, "error setting currWorldMatrixPlug value\n");

    // the multi index
    currPlug =MPlug(thisNode,aCurrMultiIndex);
    currPlug.setValue(int(multiIndex));
    SYS_ERROR_CHECK(status, "error setting currMultiIndexPlug value\n");
    
    // geometry name/type
//    currPlug =MPlug(thisNode,aCurrGeometryName);
//    currPlug.setValue(geometryName);
//    SYS_ERROR_CHECK(status, "error setting aCurrGeometryName value\n");

    currPlug =MPlug(thisNode,aCurrGeometryType);
    currPlug.setValue(geometryType);
    SYS_ERROR_CHECK(status, "error setting aCurrGeometryType value\n");

   
    /////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // execute the mel script
    //
    MString melCmd = script+"(\"" +name()+"\","+count+")";
    
    MCommandResult melResult;
    status = MGlobal::executeCommand(melCmd,melResult);
	
	// if the command did not work, then try to resource the script
	// (might have been that we were in a fresh scene and nothing was ready yet
	if (status != MS::kSuccess)
	{
		dh = block.inputValue(aScript,&status);
	    SYS_ERROR_CHECK(status, "Error getting aCmdBaseName  handle\n");    
		MString scriptFile =  dh.asString(); 	

		// try to source the script
		MString cmd = "source \"" + scriptFile+"\"";
			
		MCommandResult melResult;
		status = MGlobal::executeCommand(cmd,melResult);
		// if successfull, retry the command 
		if (!status.error())
		{
			status = MGlobal::executeCommand(melCmd,melResult);
		}
	}

	USER_ERROR_CHECK(status, "Error executing mel command, please check the function you provided is valid, error free and has the appropriate parameters!");

    // check the result type
    if ((melResult.resultType()) != (MCommandResult::kDoubleArray))
    {
        USER_ERROR_CHECK(MS::kFailure, "result of mel command has wrong type, should be doubleArray (which will be interpreted as vectorArray)!");
    }
    
    // get the result as a double array
    MDoubleArray newP;  
    status = melResult.getResult(newP);
    USER_ERROR_CHECK(status, "Error getting result of mel command!");
    
    int newCount = newP.length()/3;
    // size check
    if (newCount != count)
    {
        USER_ERROR_CHECK(MS::kFailure, "the size of the result does not match the size of the input!");
    }

    // convert the double array into a vector array
    MPointArray newPoints(newCount);
    
    for(int i=0;i<newCount;i++)
        newPoints[i]=MPoint(newP[i*3],newP[i*3+1],newP[i*3+2]);
    
    /////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // interprete and apply the result
    //


  
    // do the envelope and weights   
    if ((defEnvelope == MSD_ENVELOPE_AUTO)||((defWeights == MSD_WEIGHTS_AUTO)))
    {
        MDoubleArray envPP(count, env);
    
        if (defWeights == MSD_WEIGHTS_AUTO)
        { 
            for (int i = 0;i<count;i++)
                envPP[i] *= weights[i];
        }

        // linear interpolation between old and new points
        for (int i = 0;i<count;i++)
            newPoints[i] = (points[i] * (1-envPP[i])) + (newPoints[i] * envPP[i]);
    }


    // retransform the result if it was in world space
    if ( defSpace == MSD_SPACE_WORLD )
    {
        MMatrix worldMatrixInv = worldMatrix.inverse();
        
        for (int i = 0;i<count;i++)
            newPoints[i] *= worldMatrixInv;
    }
 
 
    // set the points    
    iter.reset();
  	for ( ; !iter.isDone(); iter.next()) 
     	iter.setPosition(newPoints[iter.index()]);    

    return status;
}


