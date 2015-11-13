/**
 * @file	PhysicalAttack.cc
 * @author	Francesco Racciatti <racciatti.francesco@gmail.com>
 * @version	0.0.1
 * @date	2015 mar 01
 */


#include "PhysicalAttack.h"

#include "Destroy.h"
#include "Move.h"


PhysicalAttack :: PhysicalAttack() : AttackBase(attack_t::PHYSICAL){

	EV_INFO << "PhysicalAttack::PhysicalAttack invoked" << endl;

}


PhysicalAttack :: ~PhysicalAttack(){

}		


void PhysicalAttack :: execute(){
	
	// execute all the actions that compose the attack
	for(size_t i = 0; i < actions.size(); i++){
			
		switch( actions[i] -> getActionType() ){
		
			case action_t::DESTROY: {
				Destroy* destroy = (Destroy*) actions[i];
				destroy -> execute();
				break;
			}
					
			case action_t::MOVE: {	
				Move* move = (Move*) actions[i];
				move -> execute();
				break;
			}
	
		} 

	}
	
}