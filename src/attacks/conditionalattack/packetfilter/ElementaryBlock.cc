/**
 * @file	ElementaryBlock.cc
 * 
 * @author	Francesco Racciatti <racciatti.francesco@gmail.com>
 */


#include "ElementaryBlock.h"
#include "seapputils.h"
#include "stdlib.h"

#include "EtherFrame.h"
#include "openflow.h"
#include "IPv4Address.h"
ElementaryBlock::ElementaryBlock(vector<string> blockElements) : FilterBlock(block_t::ELEMENTARY, blockElements)
{
}


ElementaryBlock::~ElementaryBlock()
{
}


int ElementaryBlock::getLayer() const
{
    // get the layer's name
    vector<string> fullPath = tokenize(blockElements[1], '.');
    return layertoi(fullPath[0]);
}



bool ElementaryBlock::solveFilterBlock(cMessage* msg) const
{

    // tokenize the full path to access to names of layer and fields
    vector<string> fullPath = tokenize(blockElements[1], '.');    
    
    // retrieve the encapsulated message belonging to the block's layer
    int blockLayer = layertoi(fullPath[0]);
	string actualValue;

	// <A.S>
	// if blockLayer is 0, then check for controlInfo, independent info of message layer
	if (blockLayer == 0) {
		string fieldName = fullPath[1];
	    if (msg->hasPar(fieldName.c_str())) {	    
			//retrieve the value of the parameter
			actualValue = to_string(msg->par(fieldName.c_str()).boolValue());
	 	}
	 	//OF_switch intercepts Ethernet frames. The parameter is encapsulated
	 	else { //check the existence of the parameter in the encapsulated packet
	 	    if (msg->isPacket()) {
	 	        cPacket *packet = dynamic_cast<cPacket*>(msg);
	 	        while (hasPayload(packet)) {
	 	        
	 	            //get the encapsulated packet
                    packet = packet->getEncapsulatedMsg(); 
                    
                    //check if is has the parameter
                    if (packet->hasPar(fieldName.c_str())) { 
                        actualValue = to_string(packet->par(fieldName.c_str()).boolValue());
                    }
	 	        }
	 	    }
	 	}
	}
	// <A.S>
	else {
	  	int msgLayer = getPacketLayer((cPacket*)msg);
		cMessage* encapsulatedMsg = msg;
		while (msgLayer < blockLayer) {
			encapsulatedMsg = (cMessage*)(((cPacket*)encapsulatedMsg)->getEncapsulatedPacket());
			if (encapsulatedMsg == nullptr) {
				return false;
			}
			msgLayer = getPacketLayer((cPacket*)encapsulatedMsg);
		}
		  
		// retrieve the path of fields
		vector<string> pathOfFields;
		for (size_t i = 1; i < fullPath.size(); i++) {
			pathOfFields.push_back(fullPath[i]);
		}
		
		// try to follow the path of fields until the last field is reached
		cClassDescriptor* descriptor = cClassDescriptor::getDescriptorFor(encapsulatedMsg);
		void* compoundField = encapsulatedMsg;
		int fieldIndex = -1;
		string fieldName;
		string structName;
		for (size_t i = 0; i < pathOfFields.size(); i++) {
			 fieldName = pathOfFields[i];  
			// the first entry is packet's top level field
			if (i==0) {
				fieldIndex = descriptor->findField(encapsulatedMsg, pathOfFields[i].c_str());
			}
			else {
			    if (descriptor != NULL)
			        fieldIndex = descriptor->findField(compoundField, pathOfFields[i].c_str());
			}
			  
			// field not found in the current packet
			if (fieldIndex==-1) {
				return false;
			}

            
			// all entries (except the last) refer compound fields
			if (i != pathOfFields.size()-1) {
				
				// get the pointer-to the compound field
				if (i==0) {
					structName = descriptor->getFieldStructName(encapsulatedMsg, fieldIndex);
					compoundField = descriptor->getFieldStructPointer(encapsulatedMsg, fieldIndex, 0); 		
				}
				else {
					structName = descriptor->getFieldStructName(compoundField, fieldIndex);
					compoundField = descriptor->getFieldStructPointer(compoundField, fieldIndex, 0);
				}
				  
				// get the descriptor of the compound field
				descriptor = cClassDescriptor::getDescriptorFor(structName.c_str());
				
			}
			
		}
		
		// <A.S>
        if (descriptor == NULL) {
            //the struct has not been defined in the .msg file. E.g it is defined in a .h file, so we need to know
            //the fields of the struct. The fieldIndex for the last pathOfFields is -1
            if (structName.find("oxm_basic_match") != string::npos) {
                oxm_basic_match* match = (oxm_basic_match*)compoundField; // find the beginning of the struct
                if(fieldName == "OFB_IPV4_DST") {
                    actualValue = match->OFB_IPV4_DST.str();
                }
            }
        }
        else {
		    // use the descriptor to get the last field's content
            actualValue = descriptor->getFieldAsString((cObject*)compoundField, fieldIndex, 0);
        }
	}
        
    // retrieve the comparison operator from the block
    string comparisonOperator = blockElements[2];
    // retrieve the accepted values from the block
    vector<string> acceptedValues = tokenize(blockElements[3], '|');
    // perform the comparison operation
    bool comparisonResult = false;

    for (size_t i = 0; i < acceptedValues.size(); i++) {
        //comparisonResult = evaluate(stod(actualValue), stod(acceptedValues[i]), comparisonOperator);
        comparisonResult = evaluate(actualValue, acceptedValues[i], comparisonOperator);
        if (comparisonResult == true) {
            return true;
        }
    }

    return false;
}
