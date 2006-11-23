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
#include "../include/puttyMapperNode.h"




/*************************************************************/

MTypeId puttyMapper::id( PUTTYMAPPER_NODE_ID );

     
MObject puttyMapper::aScript;   // the script to source
MObject puttyMapper::aCmdBaseName;  // the base name of the command to execute
MObject puttyMapper::aSource;           
MObject puttyMapper::aNodeReady; 
MObject puttyMapper::aScriptSourced;


MObject puttyMapper::aParticleCount; // current object 


puttyMapper:: puttyMapper() {}
puttyMapper::~puttyMapper() {}

/*************************************************************************************/
void* puttyMapper::creator() 
{ 
	return new puttyMapper(); 
}

/*************************************************************************************/
// function to mark the output dirty once one of the dynamic attributes changes
/*
MStatus puttyMapper::setDependentsDirty( const MPlug &plugBeingDirtied, MPlugArray &affectedPlugs )
{
	if ( plugBeingDirtied.isDynamic()) 
    {
		MStatus	status;
		MObject thisNode = thisMObject();

		// there is a dynamic attribute that is is dirty, so mark 
        // the outputGeometry dirty

		if ( MStatus::kSuccess == status ) 
		{
//			cerr << "\n###" << plugBeingDirtied.name();
    
			affectedPlugs.append( MPlug(thisNode,outColorPP ));
			affectedPlugs.append( MPlug(thisNode, outValuePP ));            

		}

	}
	return( MS::kSuccess );
}
*/
/*************************************************************************************/
MStatus puttyMapper::initialize()
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
 

	aParticleCount = nAttr.create( "particleCount", "pc", MFnNumericData::kInt );
	SYS_ERROR_CHECK( addAttribute( aParticleCount ), "adding aParticleCount" ); 
    

    /////////////////////////////////////////////////////////////////////////////    
    // affects
    attributeAffects( aScript ,aNodeReady);          
    attributeAffects( aSource, aNodeReady  );
    attributeAffects( aScriptSourced, aNodeReady  );
    
	attributeAffects( aScript,aScriptSourced);
	attributeAffects( aSource,aScriptSourced);
    
    attributeAffects( aParticleCount, outColorPP );
    attributeAffects( aParticleCount, outValuePP );    
    
	return MS::kSuccess;
}




/*************************************************************************************/
/*************************************************************************************/
//
//	
//
MStatus	puttyMapper::computeScriptSourced( const MPlug& plug, MDataBlock& block )
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

/*************************************************************************************/
/*************************************************************************************/

MStatus	puttyMapper::computeNodeReady( const MPlug& plug, MDataBlock& block )
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


/*************************************************************************************/
/*************************************************************************************/

MStatus puttyMapper::compute(const MPlug& plug, MDataBlock& block)
{
	MStatus status;

    //only compute if we want the output  
	if ( plug == aNodeReady )
		return computeNodeReady(plug,block);
	else if (plug == aScriptSourced)
		return computeScriptSourced(plug,block);
	else if( !((plug.attribute() == outColorPP) || (plug.attribute() == outValuePP) ))
        return( MS::kUnknownParameter) ;

	
    /////////////////////////////////////////////////////////////////////////////////////////////////
    // get inputs
	
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
    
    // count
	unsigned int count = 0;
	int nSignedPart = block.inputValue( aParticleCount ).asInt();
	if( nSignedPart > 0 )
	{
		count = nSignedPart;
	}
//    cerr << "\npm count: "<<count;
    
       
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
    
	// check the size of the result, if it is long enough to be of size vector then
    // set outColorPP with those and outValuePP with the length of the specified vectors
      
    int newCount = newF.length();


	// what's requested ?
	bool doOutColor = ( plug.attribute() == outColorPP );
	bool doOutValue = ( plug.attribute() == outValuePP );

    MVectorArray outColor;
    MDoubleArray outValue;

    
    if (newCount==count)
    {
	    // did we get back a float array
        if (doOutColor)
        {
	        outColor = MVectorArray(newCount,MVector::zero);
    	    for(int i=0;i<newCount;i++)
	    	    outColor[i].x = newF[i];
        }
         
        if (doOutValue)   
			outValue.copy(newF);            
    }
    else if ((newCount/3)==count)
    {
    	newCount /= 3;
        
    	// did we get back a vector array
        if (doOutColor)
        {
	        outColor = MVectorArray(newCount,MVector::zero);
			for(int i=0;i<newCount;i++)
    	    {
	    	    MVector vec(newF[i*3], newF[i*3+1], newF[i*3+2]);
			    outColor[i] = vec;
        	}            
        }
            
        if (doOutValue)               
        {
        	outValue = MDoubleArray(newCount);
        
	    	for(int i=0;i<newCount;i++)
    	    {
	    	    MVector vec(newF[i*3], newF[i*3+1], newF[i*3+2]);
	            outValue[i] = vec.length();
        	}
       }
    }
    else 
    {
        USER_ERROR_CHECK(MS::kFailure, "the size of the result does not match the size of the input!");
    }

    /////////////////////////////////////////////////////////////////////////////////////////
    // output the data
    
    if (doOutValue) 
    {
    	MObject outValueD = block.outputValue(outValuePP).data();
        MFnDoubleArrayData outValueFn(outValueD);
        outValueFn.set(outValue);
    }
     
    if (doOutColor) 
    {
    	MObject outColorD = block.outputValue(outColorPP).data();
        MFnVectorArrayData outColorFn(outColorD);
        outColorFn.set(outColor);
    }
     

	block.setClean( plug );

	return( MS::kSuccess );
}

