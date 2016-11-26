/*
Copyright (c) 2011, OWNER: Gérard Penet
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, 
this list of conditions and the following disclaimer.

Redistributions in binary form must reproduce the above copyright notice, 
this list of conditions and the following disclaimer in the documentation 
and/or other materials provided with the distribution.

Neither the name of the OWNER nor the names of its contributors 
may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 

IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY 
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
#pragma once

namespace skfr {

//!brief Codification of solving technics
/**
 * This codification is also the base rating of this technic 
 * (with rating = serate rating x10)
 */
enum SolvingTechnique {
    LastCell=10,				///< last cell in row column box
    SingleBox=12,               ///< single in box
    Single_R_C=15,              ///< single in row or column
    Single_after_Locked=17,		///< locked in box  clearing row/col ?? giving a fix??
    PointingClaiming=19,        ///< unknown
    HiddenPair_single=20,		///< hidden pair, hidden fix 
    NakedSingle=23,				///< cell one candidate 
    HiddenTriplet_single=25,    ///< Hidden triplet, fix
    Locked_box=26,				///< locked in box, no fix
    Locked_RC=28,				///< locked in row/col  no fix
    NakedPair=30,               ///< 2 cells containing 2 digits
    XWing=32,                   ///< XWing
    HiddenPair=34,              ///< 2 digits locked in 2 cell
    Naked_triplet=36,           ///< 3 cells containing 3 digits   
    swordfish=38,               ///< swordfish   
    HiddenTriplet=40,           ///< 3 digits in 3 cells
    XYWing=42,					///< XYWing
    XYZWing=44,					///< XYZWing
    UniqueRect1=45,             ///< UR type(s)  basic, on digit active, twins
    UniqueRect2=46,             ///< UR hidden locked setlocked set
    UniqueRect3=47,             ///< UR naked locked set
    UniqueLoop1=48,             ///< UL  locked set also URnaked quad
    UniqueLoop2=49,             ///< UL naked locked set
    NakedQuad=50,				///< 4 cells with 4 digits
    UniqueLoop3=51,             ///< UL highest rating
    Jellyfish=52,				///< jellyfish
    HiddenQuad=54,				///< 4 digits in 4 cells
    BUG=56,                     ///< in fact 5.6 to 6.1
    AlignedPairExclusion=62,    
	AIC_X_cycle=65,
    Forcing_ChainX=66,          ///<  at least 6.6 6.7 f(length)
	AIC_XY=70,
	AlignedTripletExclusion=75 ,
    NishioForcingChain=75 ,
    MultipleForcingChain=80,
    DynamicForcingChain=85,
    DynamicForcingChainPlus=90,
    NestedForcingChain=95,
	NestedLevel3=100,
	NestedLevel4=105,
	NesttedLevel5=110

};

} //namespace skfr