/**
 * @file	LocalFilter.c
 * @author	Francesco Racciatti <racciatti.francesco@gmail.com>
 */


#include "LocalFilter.h"

#include <string>

#include "PhysicalAttack.h"
#include "AttackEntry.h"
#include "Parser.h"

#include "seapputils.h"
#include "Change.h"
#include "PutMessages.h"

#include "IInterfaceTable.h"
#include "InterfaceEntry.h"
#include "InterfaceTableAccess.h"

#include "ExMachina.h"

#include "FlatNetworkConfigurator.h"
#include "IPv4NetworkConfigurator.h"

Define_Module(LocalFilter);


LocalFilter::LocalFilter()
{
	isDestroyed = false;
	injectedBytes = 0;
}


LocalFilter::~LocalFilter()
{
}


void LocalFilter::initializeGates()
{
	map<string,string> matchingGatesNames;	

	// local filter interposed between layers 5 and 4
	matchingGatesNames.insert( pair<string,string> ("app_tcp_inf$i","app_tcp_sup$o") );
	matchingGatesNames.insert( pair<string,string> ("app_tcp_sup$i","app_tcp_inf$o") );
	
	matchingGatesNames.insert( pair<string,string> ("app_sctp_inf$i","app_sctp_sup$o") );
	matchingGatesNames.insert( pair<string,string> ("app_sctp_sup$i","app_sctp_inf$o") );
	
	matchingGatesNames.insert( pair<string,string> ("app_udp_inf$i","app_udp_sup$o") );
	matchingGatesNames.insert( pair<string,string> ("app_udp_sup$i","app_udp_inf$o") );
    
	// local filter interposed between layers 4 and 3
	matchingGatesNames.insert( pair<string,string> ("udp_net_inf$i","udp_net_sup$o") );
	matchingGatesNames.insert( pair<string,string> ("udp_net_sup$i","udp_net_inf$o") );
	
	matchingGatesNames.insert( pair<string,string> ("tcp_net_inf$i","tcp_net_sup$o") );
	matchingGatesNames.insert( pair<string,string> ("tcp_net_sup$i","tcp_net_inf$o") );
	
	// local filter interposed between layers 3 and 2
	matchingGatesNames.insert( pair<string,string> ("net_ppp_inf$i","net_ppp_sup$o") );	
	matchingGatesNames.insert( pair<string,string> ("net_ppp_sup$i","net_ppp_inf$o") );	

	matchingGatesNames.insert( pair<string,string> ("net_eth_inf$i","net_eth_sup$o") );	
	matchingGatesNames.insert( pair<string,string> ("net_eth_sup$i","net_eth_inf$o") );	

	// local filter interposed between layers 2 and 1
	matchingGatesNames.insert( pair<string,string> ("ppp_phy_inf$i","ppp_phy_sup$o") );	
	matchingGatesNames.insert( pair<string,string> ("ppp_phy_sup$i","ppp_phy_inf$o") );	
	
	// local filter interposed between layers 2 and 1 [eth]
	matchingGatesNames.insert( pair<string,string> ("eth_phy_inf$i","eth_phy_sup$o") );	
	matchingGatesNames.insert( pair<string,string> ("eth_phy_sup$i","eth_phy_inf$o") );	
	
	// local filter and global filter
	matchingGatesNames.insert( pair<string,string> ("global_filter$i","global_filter$o") );	

	cGate* inputGate;
	cGate* outputGate;
	string inputGateName;
	string outputGateName;
    
	for (map<string,string>::iterator iter = matchingGatesNames.begin(); iter!=matchingGatesNames.end(); ++iter) {
        inputGateName.assign(iter->first);
        // is gate vector
		if (isGateVector(inputGateName.c_str())) {
		 	for (int i = 0; i < gateSize(inputGateName.c_str()); i++) {
				inputGate = gate(inputGateName.c_str(), i);
				// check if the gate is connected outside
                if (inputGate->isConnectedOutside()) {	
					outputGateName.assign(iter->second);
					outputGate = gate(outputGateName.c_str(),i);
					coupledGates.insert(pair<cGate*,cGate*>(inputGate, outputGate));
				}
				else {
					continue;
				}
			}
		}
		// is a single gate
        else {
			inputGate = gate(inputGateName.c_str());
			// check if the gate is connected outside
            if (inputGate->isConnectedOutside()) {
				outputGateName.assign(iter->second);
				outputGate = gate((iter->second).c_str());
				coupledGates.insert(pair<cGate*,cGate*> (inputGate, outputGate));
			}
            
			else {
				continue;
			}
		
		}
	
	}
}


void LocalFilter::initializeAttacks()
{
	cModule* node = getParentModule();
	vector<AttackEntry*> physicalAttacks;
	vector<AttackEntry*> conditionalAttacks;
	
	Parser* parser = new Parser(node);
	parser->parseConfigurationFile(attack_t::PHYSICAL, physicalAttacks);
	parser->parseConfigurationFile(attack_t::CONDITIONAL, conditionalAttacks);
	
	cMessage* selfMessage;

	//OLD
	// schedule self messages according to the occurrence time and attach to it the physical attacks
	//if( physicalAttacks.size() > 0 ){
	//	for(size_t i=0; i<physicalAttacks.size(); i++){
	//		selfMessage = new cMessage("Fire physical attack", (short) attack_t::PHYSICAL);
	//		selfMessage->addPar("attack");
	//		selfMessage->par("attack").setPointerValue(physicalAttacks[i]->getAttack());
	//		scheduleAt(physicalAttacks[i]->getOccurrenceTime(), selfMessage);
	//	}
	//}

	// NEW
	// schedule self messages according to the occurrence time and attach to it the physical attacks, 
	// except disable attack
	if (physicalAttacks.size() > 0) {
		for(size_t i=0; i<physicalAttacks.size(); i++) {
            // get the attack
            AttackBase* attack = physicalAttacks[i]->getAttack();
            
            // recognize the physical action 'disable' (physical attacks are made by a single physical action)
            ActionBase* action = attack->getAction(0);
            if (action->getActionType() == action_t::DISABLE) {
                
                // find the module 'ExMachina' in the network
                cModule* network = static_cast<cModule*>((getParentModule())->getParentModule());
                cModule* subModule = nullptr;
                bool exMachinaFound = false;
                for (cModule::SubmoduleIterator iter(network); !iter.end(); iter++) {
                    subModule = iter();
                    string className = subModule->getClassName();
                    EV << "[LocalFilter::initializeAttacks()] 'ExMachina' module found " << endl;
                    // ExMachina found
                    if (className == "ExMachina") {
                        exMachinaFound = true;
                        break;
                    }
                }
                
                // if ExMachina found
                if (exMachinaFound == true) {
                    ExMachina* exMachina = check_and_cast<ExMachina*>(subModule);
                    exMachina->scheduleDisableAttack(physicalAttacks[i]);
                }
                else {
                    opp_error("Error: ExMachina module not found in the network, please add it in the ned file");
                }
                
            }
            
            // schedule physical actions 'destroy' and 'move'
            else {
                selfMessage = new cMessage("Fire physical attack", (short) attack_t::PHYSICAL);
                selfMessage->addPar("attack");
                selfMessage->par("attack").setPointerValue(attack);
                scheduleAt(physicalAttacks[i]->getOccurrenceTime(), selfMessage);
            }
		}
	}
	
	// schedule self messages according to the occurrence time and attach to it the conditional attacks
	if( conditionalAttacks.size() > 0 ) {	
		for(size_t i = 0; i < conditionalAttacks.size(); i++){
			selfMessage = new cMessage("Fire conditional attack", (short) attack_t::CONDITIONAL);	
			selfMessage->addPar("attack");
			selfMessage->par("attack").setPointerValue(conditionalAttacks[i]->getAttack());
			scheduleAt(conditionalAttacks[i]->getOccurrenceTime(), selfMessage);
			
			
		}
	}

	// delete parser
	delete parser;	
}


LocalFilter::command_t LocalFilter::planOperation(cMessage* msg) const
{
    // check if the node is destroyed
	if (isDestroyed) {
		return command_t::DISCARD;
	}
    // the node is alive
	else {
		// check if msg is a selfMessage
        bool isSelfMessage = msg->isSelfMessage();
		if (isSelfMessage) {
			return command_t::SELFMESSAGE;
		}
        // msg is not a selfMessage
		else {
			// check the arrival gate
			string arrivalGateName = msg->getArrivalGate()->getName();
			bool isFromGlobalFilter = (arrivalGateName.find("global") != std::string::npos);
			
            // check if msg came from globalfilter
            if (isFromGlobalFilter) {
				return command_t::GLOBALFILTER;
			}
            
            // msg does not came from globalfilter
			else {
				// conditional attacks are directed only toward packets
				bool isPacket = msg->isPacket();
				if (isPacket) {
					bool isDescendingPacket = (arrivalGateName.find("sup$i") != std::string::npos);

					// add the necessary parameters to execute conditional attacks
					// add isApplicationPacket parameter
					bool isFromApp = (std::string(msg->getArrivalGate()->getFullName()).find("app")) != std::string::npos;
					bool hasParameter = msg->hasPar("isApplicationPacket");
					if (isFromApp && !hasParameter) {
						msg->addPar("isApplicationPacket");
						msg->par("isApplicationPacket").setBoolValue(true);
					        
					}
					
					// add isFiltered parameter
					hasParameter = msg->hasPar("isFiltered");
					if (isPacket && !hasParameter) {
						msg->addPar("isFiltered");
						msg->par("isFiltered").setBoolValue(false);
					}
					
					// add isToSend parameter
					hasParameter = msg->hasPar("isToSend");
					if (isPacket && !hasParameter) {
						msg->addPar("isToSend");
						msg->par("isToSend").setBoolValue(false);
					}

					// <A.S>
					// Add fromGlobalFilter parameter
					// param is attached only to the outgoing packets
					hasParameter = msg->hasPar("fromGlobalFilter");
					if (!hasParameter) {
						if(isDescendingPacket) {
							msg->addPar("fromGlobalFilter");
							// if the packet is new then add parameter and init it to false						
							if (!hasPayload(msg)) {
								msg->par("fromGlobalFilter").setBoolValue(false);   
							}
						    //else copy the value of the param of the encapsulated packet to persist consistency	
							else {
								bool value = getParamFromEncapsulatedPacket(msg, "fromGlobalFilter");
								msg->par("fromGlobalFilter").setBoolValue(value);	
							}
						}
					}
				
					bool isConditionalEnabled = enabledConditionalAttacks.size() > 0;
					if (isConditionalEnabled) {
						return command_t::CONDITIONAL;
					}
                    
					else {
						return command_t::NOATTACK;
					}
				}
				
                else {
					return command_t::NOATTACK;
				}
			}
		}
	}
}




void LocalFilter::forgeInterfaceTable()
{
    // check if the interface table exists
	IInterfaceTable *interfaceTable = InterfaceTableAccess().getIfExists();
	if (interfaceTable == nullptr) {
		return;
	}

	InterfaceEntry* interfaceEntry;
	string info;
	string interfaceName;
	int gateIndex;
	int maxEthIndex = 0;
	int numberOfEthInterfaces = 0;
	int numberOfInterfaces = interfaceTable->getNumInterfaces();
	
	// print the original interface table
	//for (int i = 0; i < numberOfInterfaces; i++) {  
	//	info.clear();
	//	interfaceEntry = interfaceTable->getInterface(i);
	//	info.append(interfaceEntry->detailedInfo());
	//	EV <<"******************** before forgeInterfaceTable() ********************" << endl;
	//	EV << info << endl;
	//}

	// couple network-layer gate index having NIC
	for (int i = 0; i < numberOfInterfaces; i++){
		interfaceName.clear();
		interfaceEntry = interfaceTable->getInterface(i);
		interfaceName = interfaceEntry->getName();
				
		// the connection order is:
		// 1. eth
		// 2. ppp
		// 3. radio
		// 4. lo0
				
		// is eth interface
		if (interfaceName.find("eth") != string::npos) {
			// remove "eth" and obtain the gate index
			interfaceName.erase(0,3);
			gateIndex = std::stoi(interfaceName);
			numberOfEthInterfaces++;
			// set the right gate index in the interface table
			interfaceEntry->setNetworkLayerGateIndex(gateIndex);		
		}
		
		// is ppp interface
		if (interfaceName.find("ppp") != string::npos) {
			// remove "ppp" and obtain the gate index
			interfaceName.erase(0,3);
			gateIndex = std::stoi(interfaceName);
			gateIndex += numberOfEthInterfaces;
			// set the right gate index in the interface table
			interfaceEntry->setNetworkLayerGateIndex(gateIndex);		
		}
        
		// TODO add other interfaces here
	
	}

	// print the forged interface table	
	//for (int i =0; i < numberOfInterfaces; i++) {
	//	info.clear();
	//	interfaceEntry = interfaceTable->getInterface(i);
	//	info.append(interfaceEntry->detailedInfo());
	//	EV << "******************** after forgeInterfaceTable() ********************" << endl;
	//	EV << info << endl;
	//}	
}


void LocalFilter::initialize(int stage)
{
	// wait the initialization of NICs
	if (stage == 4) {
		forgeInterfaceTable();
		initializeGates();
		// <A.S>
		//getNetworkParameters();
		
		initializeAttacks();

	}
}

// <A.S>
// get the network address and netmask from the 'configurator' module
//TODO change or extend for IPv4NetworkConfigurator module
void LocalFilter::getNetworkParameters() {
    if (getParentModule()->getParentModule()->getSubmodule("configurator")!= NULL) {
        if (check_and_cast<FlatNetworkConfigurator *> (getParentModule()->getParentModule()->getSubmodule("configurator")) != NULL) {
            FlatNetworkConfigurator *fc = check_and_cast<FlatNetworkConfigurator *> (getParentModule()->getParentModule()->getSubmodule("configurator"));
            networkAddr = fc->getNetworkAddress();
            netmask = fc->getNetmask();
        }
        if (check_and_cast<IPv4NetworkConfigurator *> (getParentModule()->getParentModule()->getSubmodule("configurator")) != NULL) {
            //random IPs within all ranges will be generated
            networkAddr = "";
            netmask = "";
        }
    }
    
}



void LocalFilter::handleMessage(cMessage* msg)
{
	//EV << "LocalFilter::handleMessage has intercepted a message" << endl;
    
    double delay = 0.0;	
	// choose the action to perform
	switch (planOperation(msg)) {
	
		// the node is destroyed then discard all messages
		case command_t::DISCARD: {
			delete msg;
			return;
		}
		
		// the received message is a self message
		case command_t::SELFMESSAGE: {
			attack_t msgKind = (attack_t) msg->getKind(); 	
	
			switch(msgKind) {
		
				// immediatly execute physical attacks
				case attack_t::PHYSICAL: {		
					PhysicalAttack* physicalAttack = (PhysicalAttack*) (msg->par("attack").pointerValue());
					physicalAttack->execute();			
					break;
				}
				
				// enable conditional attacks (to execute them must also be satisfied the filter) 
				case attack_t::CONDITIONAL: {
					ConditionalAttack* conditionalAttack = (ConditionalAttack*) (msg->par("attack").pointerValue());
					// <A.S>
					conditionalAttack->setNetworkParameters(networkAddr, netmask);
					enabledConditionalAttacks.push_back(conditionalAttack);
					break;
				}
				
			}
			
			delete msg;
			return;
		}
		
		
		case command_t::GLOBALFILTER: {
			// local filter receives only PutReq
			if (isPutReq(msg)) {

				PutReq* putReq = nullptr;
				cMessage* carriedPacket = nullptr;
				
				direction_t direction;
								
				putReq = (PutReq*)(msg);
				
				// check if the carried msg is a packet
				carriedPacket = putReq->getMsg();
				
				if (!carriedPacket->isPacket()) {
					//EV << "LocalFilter::handleMessage the received PutMsg doesn't carry a packet" << endl;
					delete msg;
					return;
				}

				// make a hard copy of the carried packet (putReq will be destroyed)
				carriedPacket = hardCopy((cPacket*)(carriedPacket));

				/* //forge the sending data
				forgeSendingData(carriedPacket);
				direction = putReq->getDirection();
				 //handle the carriedPacket
				switch (direction) {
				
					case direction_t::TX: {
					
						cGate* arrivalGate;
						arrivalGate = carriedPacket->getArrivalGate();
						send(carriedPacket, coupledGates[arrivalGate]); 
						break;
						// call forgeSendingData!!!
					
					}
					
					case direction_t::RX: {
						
                      break;
					}			
				
				}*/
				
				string outputGateOverallName;				
				string outputGateName;
				vector<string> tokens;
				int outputGateIndex;
				cGate* outputGate;
								
				// get the output gate overall name
				outputGateOverallName.assign(carriedPacket->par("outputGate").stringValue());

				// find the output gate
				tokenize(tokens, outputGateOverallName, '[');				
				outputGateName.assign(tokens[0]);
				// remove last char ']'
				tokens[1].pop_back();
				outputGateIndex = atoi(tokens[1].c_str());
				outputGate = gate(outputGateName.c_str(), outputGateIndex);
				send(carriedPacket, outputGate);
			}
            delete msg;
            return;	
			
		}
		
		
		case command_t::CONDITIONAL: {
			cGate* arrivalGate;
			vector<cMessage*> generatedPackets;
			vector<double> delays;

			// perform all enabled conditional attacks
			for (size_t i = 0; i < enabledConditionalAttacks.size(); i++) {
						
				generatedPackets.clear();
				delays.clear();
			
				// packet filter match
				if (enabledConditionalAttacks[i]->matchPacketFilter(msg)) {							
					enabledConditionalAttacks[i]->execute(&msg, generatedPackets, delays, delay);								
					
					// send the original packet if not dropped 
					if (msg != nullptr) {
						// TODO check if out interfaces then reset isFiltered
						arrivalGate = msg->getArrivalGate();
						sendDelayed(msg, delay, coupledGates[arrivalGate]);
					}
					else {
						EV << "LocalFilter::handleMessage has drop the original packet" << endl;
					}
					
					for (size_t i = 0; i < generatedPackets.size(); i++) {
						
						// the generated packet was destroyed after its creation
						if (generatedPackets[i] == nullptr) {
							continue;
						}
						else{
							// check if put message	
							if ( isPutMsg(generatedPackets[i]) ) {
								send(generatedPackets[i], "global_filter$o");
								continue;
							}
							else {
								// TODO remove this check, it is useless because isToSend is always present
								bool hasParameter = false;
								hasParameter = generatedPackets[i]->hasPar("isToSend");

								if (!hasParameter) {
									string errorMsg;
									errorMsg.append("LocalFilter::handleMessage can't find isToSend parameter");
									opp_error(errorMsg.c_str());
								}
							
								// forge sending data
								forgeSendingData(generatedPackets[i]);
							
								bool isToSend = generatedPackets[i]->par("isToSend").boolValue();
								if (!isToSend) {
									continue;
								}
										
								generatedPackets[i]->par("isToSend").setBoolValue(false);	
								
								arrivalGate = generatedPackets[i]->getArrivalGate();
								sendDelayed(generatedPackets[i], delays[i], coupledGates[arrivalGate]); 
								
							}
						}
					}
					
					return;
					
				}
				// packet filter does not match
				else{
					arrivalGate = msg->getArrivalGate();
					send(msg, coupledGates[arrivalGate]);			
				}
				
			}

			return;
		}
		
			
		case command_t::NOATTACK: {
			cGate* arrivalGate = msg->getArrivalGate();
			send(msg, coupledGates[arrivalGate]);
			return;
		}
			
	}
	
	// EV << "LocalFilter::handleMessage ended succesfully" << endl;
}


void LocalFilter::forgeSendingData (cMessage* msg)
{
	bool hasParameter = msg->hasPar("outputGate");
	if (hasParameter == true) {
        string outputGateOverallName;
        vector<string> tokens;
        string outputGateName;
        int outputGateIndex;
        cGate* outputGate;
        cGate* inputGate;
        int inputGateId;
        cGate* previousGate;
        int previousGateId;
        cModule* previousModule;

        // get the output gate overall name
        outputGateOverallName.assign(msg->par("outputGate").stringValue());

        // find the output gate
        tokenize(tokens, outputGateOverallName, '[');
        outputGateName.assign(tokens[0]);
        // remove last char ']'
        tokens[1].pop_back();
        outputGateIndex = atoi(tokens[1].c_str());
        outputGate = gate(outputGateName.c_str(), outputGateIndex);

        // find the relative input gate
        map<cGate*, cGate*>::iterator iter;
        for (iter = coupledGates.begin(); iter != coupledGates.end(); ++iter) {
            if (iter->second == outputGate) {
                inputGate = iter->first;
                break;
            }
        }
        
        inputGateId = inputGate->getId();
        
        // find the previous module (i.e. the module connected to the input gate)
        previousGate = inputGate->getPreviousGate();
        previousModule = check_and_cast<cModule*>(previousGate->getOwner());
        previousGateId = previousGate->getId();
        
        // forge sending data
        msg->setSentFrom(previousModule, previousGateId, simTime() );
        msg->setArrival(this, inputGateId);
	}
    
	else {
		EV << "LocalFilter::forgeSendingData hasn't find the outputGate parameter or the parameter is 'none'" << endl;
	}
}






