/* COPYRIGHT --
 *
 * This file is part of puttyNodes, a collection of utility nodes for Autodesk Maya.
 * puttyNodes is (c) 2006 Carsten Kolve <carsten@kolve.com>
 * and distributed under the terms of the GNU GPL V2.
 * See the ./License-GPL.txt file in the source tree root for more information.
 *
 * THIS PACKAGE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES
 * OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */


// maya includes

#include <maya/MIOStream.h>
#include <math.h>
#include <stdlib.h>

#include <maya/MVectorArray.h>
#include <maya/MDoubleArray.h>
#include <maya/MIntArray.h>
#include <maya/MMatrix.h>
#include <maya/MGlobal.h>

#include <maya/MFnDependencyNode.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnUnitAttribute.h>
#include <maya/MFnVectorArrayData.h>
#include <maya/MFnDoubleArrayData.h>
#include <maya/MFnArrayAttrsData.h>
#include <maya/MFnStringData.h>
#include <maya/MFnMatrixData.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnCompoundAttribute.h>


#include "../include/helperMacros.h"
#include "../include/puttyNodeIds.h"
#include "../include/puttyEmitterNode.h"


/****************************************************************************/
// all custom attributes here
MTypeId puttyEmitter::id( PUTTYEMITTER_NODE_ID );

MObject  puttyEmitter::aDaInput;        // storage for the pp data to be emitted
MObject  puttyEmitter::aDaInputName;    
MObject  puttyEmitter::aDaInputData;    

MObject  puttyEmitter::aVaInput;        // storage for the pp data to be emitted
MObject  puttyEmitter::aVaInputName;    
MObject  puttyEmitter::aVaInputData;   


/****************************************************************************/
// constructor
puttyEmitter::puttyEmitter()
{}

/****************************************************************************/
// destructor
puttyEmitter::~puttyEmitter()
{}

/****************************************************************************/
void *puttyEmitter::creator()
{
    return new puttyEmitter;
}

/****************************************************************************/
//	Initialize the node, create user defined attributes.
MStatus puttyEmitter::initialize()
{
	// all the attributes of the puttyEmitter will be a combination of
    // dynamic attributes connected to an "internal" multi compound
    // which consists of a string (the name of the attibute) and a storage
    // for the double/vector array
    // the string name will be used when it comes to injecting data into the output
    // of the emitter

	/////////////////////////////////////////////////
    // da
	// create compound of doubleArrays and string
	MFnTypedAttribute tAttr;
	MFnCompoundAttribute cAttr;    
    
    // attribute for the name
	aDaInputName =  tAttr.create( "daInputName", "dain", MFnData::kString,NULL );
	tAttr.setReadable( true ); 
	tAttr.setWritable( true ); 
	tAttr.setStorable( true ); 
    SYS_ERROR_CHECK( addAttribute( aDaInputName ), "adding aDaInputName" );  

	// the array    
	aDaInputData =  tAttr.create( "daInputData", "daid", MFnData::kDoubleArray,NULL);
	tAttr.setReadable( true ); 
	tAttr.setWritable( true ); 
	tAttr.setStorable( true ); 
    SYS_ERROR_CHECK( addAttribute( aDaInputData ), "adding aDaInputName" );  
         
    // compound
	aDaInput = cAttr.create("daInput", "dai");
    cAttr.addChild(aDaInputName);
    cAttr.addChild(aDaInputData); 
    cAttr.setArray(true);
	SYS_ERROR_CHECK( addAttribute( aDaInput ), "adding aDaInput" );   

	/////////////////////////////////////////////
    // va
    // attribute for the name
	aVaInputName =  tAttr.create( "vaInputName", "vain", MFnData::kString,NULL );
	tAttr.setReadable( true ); 
	tAttr.setWritable( true ); 
	tAttr.setStorable( true ); 
    SYS_ERROR_CHECK( addAttribute( aVaInputName ), "adding aVaInputName" );  

	// the array    
	aVaInputData =  tAttr.create( "vaInputData", "vaid", MFnData::kVectorArray,NULL);
	tAttr.setReadable( true ); 
	tAttr.setWritable( true ); 
	tAttr.setStorable( true ); 
    SYS_ERROR_CHECK( addAttribute( aVaInputData ), "adding aVaInputData" );  
         
    // compound
	aVaInput = cAttr.create("vaInput", "vai");
    cAttr.addChild(aVaInputName);
    cAttr.addChild(aVaInputData); 
    cAttr.setArray(true);
	SYS_ERROR_CHECK( addAttribute( aVaInput ), "adding aVaInput" );   
        
    // affects
    attributeAffects(aDaInputName, mOutput);
    attributeAffects(aDaInputData, mOutput);
    attributeAffects(aDaInput, mOutput);     
    
    attributeAffects(aVaInputName, mOutput);
    attributeAffects(aVaInputData, mOutput);
    attributeAffects(aVaInput, mOutput);          
    
	return( MS::kSuccess );
}

/****************************************************************************/
//  Call emit emit method to generate new particles.
MStatus puttyEmitter::compute(const MPlug& plug, MDataBlock& block)
{
	MStatus status;

	// Determine if we are requesting the output plug for this emitter node.
	//
	if( !(plug == mOutput) )
        return( MS::kUnknownParameter );

	// Get the logical index of the element this plug refers to,
	// because the node can be emitting particles into more 
    // than one particle shape.
	//
	int multiIndex = plug.logicalIndex( &status );
	SYS_ERROR_CHECK(status, "ERROR in plug.logicalIndex.\n");

	// Get output data arrays (position, velocity, or parentId)
	// that the particle shape is holding from the previous frame.
	//
	MArrayDataHandle hOutArray = block.outputArrayValue( mOutput, &status);
	SYS_ERROR_CHECK(status, "ERROR in hOutArray = block.outputArrayValue.\n");

	// Create a builder to aid in the array construction efficiently.
	//
	MArrayDataBuilder bOutArray = hOutArray.builder( &status );
	SYS_ERROR_CHECK(status, "ERROR in bOutArray = hOutArray.builder.\n");

	// Get the appropriate data array that is being currently evaluated.
	//
	MDataHandle hOut = bOutArray.addElement(multiIndex, &status);
	SYS_ERROR_CHECK(status, "ERROR in hOut = bOutArray.addElement.\n");

	// Create the data and apply the function set,
	// particle array initialized to length zero, 
	// fnOutput.clear()
	//
	MFnArrayAttrsData fnOutput;
	MObject dOutput = fnOutput.create ( &status );
	SYS_ERROR_CHECK(status, "ERROR in fnOutput.create.\n");

	// Check if the particle object has reached it's maximum,
	// hence is full. If it is full then just return with zero particles.
	//
	bool beenFull = isFullValue( multiIndex, block );
	if( beenFull )
	{
//		return( MS::kSuccess );
	}

	// Get deltaTime, currentTime and startTime.
	// If deltaTime <= 0.0, or currentTime <= startTime,
	// do not emit new pariticles and return.
	//
/*	MTime cT = currentTimeValue( block );
	MTime sT = startTimeValue( multiIndex, block );
	MTime dT = deltaTimeValue( multiIndex, block );
	if( (cT <= sT) || (dT <= 0.0) )
	{
		// We do not emit particles before the start time, 
		// and do not emit particles when moving backwards in time.
		// 

		// This code is necessary primarily the first time to 
		// establish the new data arrays allocated, and since we have 
		// already set the data array to length zero it does 
		// not generate any new particles.
		// 
		hOut.set( dOutput );
		block.setClean( plug );

		return( MS::kSuccess );
	}

*/
    int plugIndex = plug.logicalIndex( &status );

	// get all the double array attributes from the multi and add them to the output
    //
	MArrayDataHandle hDaInArray = block.inputArrayValue( aDaInput, &status);
	SYS_ERROR_CHECK(status, "ERROR in hDaInArray = block.inputArrayValue.\n");
	
    hDaInArray.jumpToElement(0);
    
    for (int i=0; i < hDaInArray.elementCount();i++)
    {
    	// get handles to children from the current array element
        MDataHandle hDaElement = hDaInArray.inputValue(&status);
		SYS_ERROR_CHECK(status, "ERROR in hDaElement = hDaInArray.inputValue.\n");        
    	
        MDataHandle hDaElementName = hDaElement.child(aDaInputName);
        MDataHandle hDaElementData = hDaElement.child(aDaInputData);        
        
        // now get the data from the handles
        MFnStringData nameFn(hDaElementName.data());
        MString name = nameFn.string(&status);
		SYS_ERROR_CHECK(status, "ERROR in name = nameFn.string.\n");                
        
//        cerr << "\n-----------\nname: " << name ;
		// if we don't have a name, ignore this element
        if (name != "")
		{
            
        	MFnDoubleArrayData arrayFn(hDaElementData.data());
	        MDoubleArray array = arrayFn.array(&status);

            if (status.error())
            	return status;
                
//			SYS_ERROR_CHECK(status, "ERROR in da array = arrayFn.array.\n"); 
	
//            cerr << "\narray: " << array ;       
             
        	// put it all in the output array
	        MDoubleArray output  = fnOutput.doubleArray(name, &status);
            output.copy(array);
        }
        
        hDaInArray.next();
    }
    

	// now do the same for the va data
    //
	MArrayDataHandle hVaInArray = block.inputArrayValue( aVaInput, &status);
	SYS_ERROR_CHECK(status, "ERROR in hVaInArray = block.inputArrayValue.\n");
	
    hVaInArray.jumpToElement(0);
    
    for (int i=0; i < hVaInArray.elementCount();i++)
    {
    	// get handles to children from the current array element
        MDataHandle hVaElement = hVaInArray.inputValue(&status);
		SYS_ERROR_CHECK(status, "ERROR in hVaElement = hVaInArray.inputValue.\n");        
    	
        MDataHandle hVaElementName = hVaElement.child(aVaInputName);
        MDataHandle hVaElementData = hVaElement.child(aVaInputData);        
        
        // now get the data from the handles
        MFnStringData nameFn(hVaElementName.data());
        MString name = nameFn.string(&status);
		SYS_ERROR_CHECK(status, "ERROR in name = nameFn.string.\n");                
        
		// if we don't have a name, ignore this element
        if (name != "")
		{
            
        	MFnVectorArrayData arrayFn(hVaElementData.data());
	        MVectorArray array = arrayFn.array(&status);
            if (status.error())
            	return status;

//			SYS_ERROR_CHECK(status, "ERROR in da array = arrayFn.array.\n"); 
             
        	// put it all in the output array
	        MVectorArray output  = fnOutput.vectorArray(name, &status);
            output.copy(array);
        }
        
        hVaInArray.next();
    }


	// Update the data block with new dOutput and set plug clean.
	//
	hOut.set( dOutput );
	block.setClean( plug );

	return( MS::kSuccess );
}


