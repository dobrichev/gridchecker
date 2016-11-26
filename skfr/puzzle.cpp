/*
Copyright (c) 2011, OWNER: Gérard Penet
All rights reserved.stribution and use in source and binary forms, with or without modification, 
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

#include "flog.h"				// relay for printing only in tests
#include "opsudo.h"				// storing and managing options
#include "puzzle.h"				// general class to solve a puzzle
#include "utilities.h"

namespace skfr {

const int chx_max =9; //max size  for an event set could be a variable parameter?
                   // limit could be 20 in zcx, but must be checked to go over 9

const char *orig[]={"row ","column ","box "," "};
const char *lc="ABCDEFGHI";
const char *orig1="RCB ";

#include "puzzle_globals.cpp"


PUZZLE::PUZZLE() {
	solution = gsolution.pg;  
	T81 = &tp8N;
	T81C = &tp8N_cop;
	T81t = T81->t81;
	T81tc = T81C->t81;

	tp8N.SetParent(this,&EE);
	yt.SetParent(this,&EE);
	tchain.SetParent(this,&EE);
	T81dep.SetParent(this,&EE);
	tp8N.SetParent(this,&EE);
	zpaires.SetParent(this,&EE);
	tevent.SetParent(this,&EE);
 	ur.SetParent(this,&EE,T81t,T81tc,
		   alt_index.tchbit.el);
	zpln.SetParent(this,&EE);
    zcf.SetParent(this,&EE);
    zcxb.SetParent(this,&EE);
    zcx.SetParent(this,&EE);
}

/* added here routines preliminary in Bitfields
   images
   */
void PUZZLE::ImagePoints(BF81 & zz) const {
	char s0[5], s1[5];
	int ns = 0, mode = 0;
	for(int i = 0; i < 81; i++) {
		if(zz.On(i)) {
			strcpy_s(s1, 5, cellsFixedData[i].pt);
			switch (mode) {
				case 0:
					mode = 1;
					break;
				case 1: 
					if(s0[1]==s1[1]) {  // ligne r1c12 on empile colonnes
						EE.E(s0);
						EE.E(s1[3]);
						mode = 2;
					}
					else if(s0[3] == s1[3]) { // colonne r12c1
						EE.E(s0[0]);
						EE.E(s0[1]);
						EE.E(s1[1]);
						mode = 3;
					}
					else
						EE.E(s0);
					break;
				case 2:
					if(s0[1]==s1[1])EE.E(s1[3]);  //suite ligne
					else mode = 1;  break;
				case 3:
					if(s0[3]==s1[3]) EE.E(s1[1]);//suite colonne  insertion ligne
					else {mode = 1; EE.E(&s0[2]);}
					break;
			}  // end switch
			strcpy_s(s0, 5, s1);
		}  // end if(On(i))
	}
	switch (mode) { // finir le traitement
		case 1:
			EE.E(s0);
			break;
		case 3:
			EE.E(&s0[2]);
			break;
	} // end switch
}

//------
void PUZZLE::ImageCand(BFCAND &zz, char *lib) const {
	if(!options.ot)
		return;
	EE.E(lib);   
	for(int i = 1; i < zpln.ip; i++) {
		if(zz.On(i)) {
			zpln.Image(i);
			EE.Esp();
		}
	}
	EE.Enl();
}

//------
void PUZZLE::GetCells(BFCAND & zz,BF81 &cells) const {
	for(int i = 1; i < zpln.ip; i++) {
		if(zz.On(i)) {
			cells.Set(zpln.zp[i].ig);
		}
	}
	EE.Enl();
}

//------
void PUZZLE::Image(const BFTAG & zz, const char * lib, int mmd) const {
	if(!options.ot)
		return;
	EE.E(lib);   
	if(mmd)
		zpln.ImageTag(mmd);
	EE.E(" : ");
	for(int i = 2; i <col ; i++) {
		if(zz.On(i)) {
			zpln.ImageTag(i);
			EE.Esp();
		}
	}
	EE.Enl();
}

void PUZZLE::Image(const SQUARE_BFTAG & zz) const{
	EE.Enl( "SQUARE_BFTAG Image");
	for(int i=2;i< col;i++)  
		if(zz.t[i].IsNotEmpty())  
			Image(zz.t[i]," ",i); 
}

void PUZZLE::Elimite(const char * lib) 
   { stop_rating=1;
    if(!options.ot) return;
	EE.Enl2();
	EE.E("table:"); 
	EE.E(lib); 
	EE.E("limite atteinte ");
	EE.Enl2();
}

void PUZZLE::Estop(const char * lib) {
	stop_rating=1;
	if(!options.ot)
		return;
	EE.Enl(lib); 
	EE.Enl();
}

int PUZZLE::is_ed_ep() {  // at the start of a new cycle
	if(cycle<2) return 0;
	c_ret = Is_ed_ep_go();
	return c_ret;
}

void PUZZLE::SetEr() {  // something found at the last difficulty level  
	if((cycle==1)&& difficulty>edmax) edmax=difficulty;
	if(((!assigned)|| (!epmax))&& difficulty>epmax) epmax=difficulty;
	if(difficulty>ermax) ermax=difficulty;      
}

void PUZZLE::Seterr(int x) {  // an error condition has been found
	ermax=0;
	epmax=0;
	edmax=x;
}

// filter at the start of a new cycle

int PUZZLE::Is_ed_ep_go() {  // is the ed or  condition fullfilled
	switch(options.o1) {
	case 0:
		return 0;       // nothing to do

		// if -d command, other filters ignored  
	case 1:
		if((ermax- edmax) > options.delta) {
			ermax = 0;
			epmax = 0;
			return 1;
		}  // not diamond
		return 0;// continue if still ed


		// if -p command, similar results
	case 2:
		if(!assigned)
			return 0;   // -p command
		if((ermax- epmax) > options.delta)	{
			ermax = 0;
			return 1;
		} // not pearl
		return 0;// continue if still ep
	}	

	// now, we have no -d no -p command but at least one filter is on	
	// give priority to the -n() command

	if(options.filters.On(3))// -n() command
		if(ermax >= options.miner) {
			ermax = 0;
			return 3;
		} // finished
		else if(cycle > options.edcycles)
			return 4;

	//then all max conditions
	if(edmax >= options.maxed || epmax >= options.maxep || ermax >= options.maxer) {
		ermax = 0;
		return 3;
	} // finished

	// and finally min ED and min EP
	if(edmax <= options.mined) {
		ermax = 0;
		return 3;
	} // finished
	if(assigned && epmax <= options.minep) {
		ermax = 0;
		return 3;
	} // finished
	if(!options.os)
		return 0; // finish with split ok

	// that sequence should work for any combinaison of filters.
	if((options.filters.f & 7) == 1)
		return 4; // ed ok for split   	
	if(assigned && ((options.filters.f & 6) == 2))
		return 4; // ep ok for split   	

	return 0;
}

// the next routine is called at each step of the process
// the searched rating is stored in "difficulty"
// a check of filters is made and if any special filter is active,
// the process is cancelled
// return code 0 nothing special
//             1 process cancelled
//             2 ignore (er<x.y) these routines

void PUZZLE::Step(SolvingTechnique dif) {   // analyse what to do at that level
	rating_ir = 0;
	difficulty = dif;
	if(options.o1 < 2)
		return; //nothing to do for -d command
	// if -p command, stop if we pass maxep
	if(options.o1 == 2) {
		if(assigned && difficulty > options.maxep) {
			ermax = 0;
			rating_ir = 1;
			return;
		} 
		else
			return;
	}
	// now other special filters 
	// -n() active if we pass the limit
	if(options.filters.On(3)) {    // -n() command
		if(difficulty >= options.miner) {
			ermax = 0;
			rating_ir = 3;
			return;
		}  // finished
	}
	if(difficulty >= options.maxer) {
		ermax = 0;
		rating_ir = 2;
		return;
	} // filter on maxer cancelling high rating

	/*
	if(difficulty>maxed || difficulty>maxep ){maxer=0; ir=1;return;}
	*/
}

void PUZZLE::Copie_T_c() {
	tp8N_cop = tp8N;
	for(int i = 0; i < 9; i++)
		c_cop[i]=c[i];
}

void PUZZLE::Actifs() {
	zactif.SetAll_0();
	T81->Actifs(zactif);
	//for(int i = 0; i < 27; i++)
	//	elza81[i] = divf.elz81[i] & zactif;
}

void PUZZLE::cInit(int un) {
	for(int i = 0; i < 9; i++)
		if(un)
			c[i].SetAll_1();
		else
			c[i].SetAll_0();
}

void PUZZLE::cFixer(int ich, int i8) {
	for(int i = 0; i < 9; i++)
		c[i].Clear(i8); // pas de candidat ici
	c[ich] -= cellsFixedData[i8].z;
}   // ni en zone influence

void PUZZLE::cReport() {    // on charge cand de ztzch
	for(int i8 = 0; i8 < 81; i8++) {
		CELL *p8 = &tp8N.t81[i8];
		if(p8->v.typ)
			continue;
		p8->v.ncand = 0;
		p8->v.cand.f = 0;
		p8->scand[0] = 0;
		for(int i = 0; i < 9; i++)
			if(c[i].On(i8))
				p8->v.cand.Set(i);
		p8->v.ncand = p8->v.cand.CountEtString(p8->scand);
	}
}

int PUZZLE::Recale() {
	//cReport();
	nfix = 0;
	for(int i = 0; i < 81; i++) {
		fix[i] = '0';
		if(T81t[i].v.ncand == 0)
			return 0;
	}
	alt_index.Genere(T81t);   
	for(int i = 0; i < 27; i++)
		for(int j = 0; j < 9; j++)
			if(alt_index.tchbit.el[i].eld[j].n == 0)
				return 0;
	return 1;
}

int PUZZLE::Directs() { //en tete appliquer regle de base
	int ir = 0, i;
	for(i = 0; i < 81; i++) {
		if((!T81t[i].v.typ) && (T81t[i].v.ncand == 1)) {   // case 1 candidat
			FixerAdd(i, T81t[i].scand[0], 3);
			ir = 1;
		}
	}
	for(i = 0; i < 27; i++) {
		for(int j = 0; j < 9; j++) {  // chiffre une place
			if(alt_index.tchbit.el[i].eld[j].n == 1) {
				int k = alt_index.tchbit.el[i].eld[j].b.First(),
					//i8 = divf.el81[i][k];
					i8 = cellsInGroup[i][k];
				if(!T81t[i8].v.typ)	{
					FixerAdd(i8,(char)('1' + j), i/9);
					ir = 1;
				}
			}
		}
	}
	return ir;
}

// doing assignments matching expected rating if any
//    LastCell=10,		 last cell in row column box
//    SingleBlock=12,    single box
//    Single_R_C=15,     single row column
//    NakedSingle=23,	 cell one candidate 

int PUZZLE::FaitDirects(int rating) {
	if(stop_rating)
		return 1; 
	int ir = 0;
	for(int i = 0; i < 81; i++) {
		char c1 = fix[i], c2 = fixt[i];
		if(c1 - '0') {     // donc fixée
			// filter on rating expected
			int ok = 0;
			const CELL_FIX &p = cellsFixedData[i];
			switch(rating) {
			case 10:
				if((DIVF::N_Fixes(gg.pg,p.el) == 8) 
					|| (DIVF::N_Fixes(gg.pg,p.pl+9) == 8)
					|| (DIVF::N_Fixes(gg.pg,p.eb+18) == 8))
					ok = 1;
				break;
			case 12:
				if(alt_index.tchbit.el[p.eb + 18].eld[c1 - '1'].n == 1)
					ok = 10;
				break;
			case 15:
				if(alt_index.tchbit.el[p.el].eld[c1 - '1'].n == 1)
					ok = 1;
				if(alt_index.tchbit.el[p.pl + 9].eld[c1 -'1'].n == 1)
					ok = 1;
				break;
			case 23:
				if((gg.pg[i] == '0') && (T81t[i].v.ncand == 1))
					ok = 1;
				break;
			}
			if(ok)
				ir += FaitGo(i, c1, c2); 
		}
	}
	EE.Enl();
	if(ir)
		cReport();
	return ir;
}

int PUZZLE::FaitGo(int i, char c1, char c2) { // function also called if single forced
	EE.E(++assigned);
	EE.E(" ");
	EE.E(cellsFixedData[i].pt);
	EE.E("=");
	EE.E(c1);
	EE.E(" ");
	if(c2 < 4)
		EE.Enl(orig1[c2]);
	else EE.Enl(" assigned");
	if((solution[i] - c1) && (!stop_rating)) { // validite  fixation
		stop_rating=1;
		EE.E( "FIXATION INVALIDE");
		return 0;
	}
	T81->Fixer(c1 - '1', i, 1);
	gg.pg[i] = c1;
	return 1;
}

//----                     supprimer ch en elem sauf  pos
int PUZZLE::ChangeSauf(int elem, BF16 pos, BF16 chiffres) {
	int ir=0;
	for(int i = 0; i < 9; i++) {
		if(pos.On(i))
			continue;
		ir += T81t[cellsInGroup[elem][i]].Change(chiffres, this);
	}
	return ir;
}

//----                     garder  ch en elem   pos
int PUZZLE::Keep(int elem, BF16 pos, BF16 chiffres) {
	int ir=0;
	for(int i=0;i<9;i++) {
		if(pos.On(i))
			ir += T81t[cellsInGroup[elem][i]].Keep(chiffres, this);
	}
	return ir;
}


//------                  ou simple deux points quelconques
int PUZZLE::Keep(int ch1, USHORT p1, USHORT p2) {
	BF81 ze = cellsFixedData[p1].z;
	ze &= cellsFixedData[p2].z;
	ze &= c[ch1];
	if(ze.IsNotEmpty())
		return T81->Clear(ze, ch1);
	return 0;
}

//---------
int PUZZLE::NonFixesEl(int el) {
	int n = 0;
	for(int i = 0; i < 9; i++)
		if(gg.pg[cellsInGroup[el][i]] == '0')
			n++;
	return n;
}

int PUZZLE::CheckChange(int i, int ch) {
	if(stop_rating) return 1;
	if(solution[i]-(ch + '1'))
		return 0;
	stop_rating = 1;
	EE.E("ELIMINATION INVALIDE ");
	EE.E(ch+1);
	EE.Enl(cellsFixedData[i].pt);
	return 1;
}

void PUZZLE::PointK() {
	couprem++;
	EE.E( "CREM=" );
	EE.E(couprem );
}

/* comments on the process
level 2 base 9.5   Forcing chain authorized
level 3 base 10.0  lev 2 + multiple chains authorized
*/

//==============================================
// 95 Quick is a partial processing for nested + forcing chains
//    and nested + multi chains
// can be used to reduce the number of puzzles when looking for hardest puzzles

void PUZZLE::InitNested() { // common part before starting nested processing
	// lock the start situation with fully expanded  hdp from step 90
	if(options.ot) {
		EE.Enl("Init  nested levels  ");
	}
	opp = 0;
	zcf.h.dp = zcf.hdp_dynamic; // restore the index in zcf 
	zcf.ExpandAll();// not derived weak links
	zcf.LockNestedOne();
	zcx.LockNestedOne();
	zcxb.LockNestedOne();
	//zcf.h_one.d.Image();
}

int PUZZLE::Rating_baseNest(USHORT base, int quick) {
	tchain.SetMaxLength(base);
	rbase = base;
	opdiag = 0;
	if(options.ot) {
		EE.E("start  nested levels base =");
		EE.Enl(base);
		if(quick)
			EE.Enl("Quick mode is On");
		else
			EE.Enl("Quick mode is Off");
		if(rbase>90){
			PointK(); // milestone for debugging
			EE.Enl();
	    }
	}

	d_nested = d_nested2 = d_nested8 = zcf.h_one.d;
	for(int i = 2; i < col; i++) {
		 
		// first step in the search for nested chains
        // fill zcf.h_nest.d.t[i]  with the full expansion
        // in that mode

		GoNestedTag(i);
	}   

	if(0){
		EE.E("end phase 1");
		Image( d_nested );
	}


	// we have now fully expanded tags in d_nested2 or d_nested
	// we look for potential eliminations

	Rbn_Elims(d_nested2.t, 1);
//	if(rbn_elimt.IsEmpty())
	if(rbn_elimt.Count()<5)
		Rbn_Elims(d_nested8.t, 2);
//	if(rbn_elimt.IsEmpty())
  	if(rbn_elimt.Count()<5)
		Rbn_Elims(d_nested.t, 3);

	if(rbn_elimt.IsEmpty())
		return 0;

	// here a test to locate discrepancies with final 
	// produce a detailed analysis for first case in dual mode 
	// test neutral after bug fixed
	if(0 && rbn_it2) { // some case 2 to apply

		//opdiag=1;

	    d_nested=d_nested2 = zcf.h_one.d; // restore the start

		int i = rbn_t2[0];
		GoNestedTag(i);
		GoNestedTag(i ^ 1);
		//opdiag = 0;
	}


	// if we are in quick mode, set all elims with rating base+ .3/.5
	// process as quick mode if achieved rating is high enough
	if(quick || (base+10+(base-95)/5)<ermax) {
		Image(rbn_elimt,"quick elim potential", 0);
		int j = 3;
		for(int i = 3; i < col; i += 2) { // first in tchain mode
			if(rbn_elimt.On(i)) {
				j = i;
				tchain.LoadChain(base + 3, "quick nested ", i >> 1);
				rbn_elimt.Clear(i);
				break;
			}
		}
		for(int i = j + 2; i < col; i += 2) { // others in direct mode
			if(rbn_elimt.On(i)) {
				zpln.Clear(i >> 1);
			}
		}
		return Rating_end(200);
	}


	// not quick mode, go step by step to find the lowest rating
	if(rbase>100 && options.ot) {
		EE.E("action it2 =");
		EE.E(rbn_it2);
		EE.E(" itch =");
		EE.E(rbn_itch);
		EE.Enl(); 
	}

	if(rbn_elims1.IsNotEmpty()) {  // some case 1 to apply
		for(int i = 2; i < col; i += 2) {
			if(rbn_elims1.On(i)) {
				GoNestedCase1(i >> 1);
            if(stop_rating) return 1;// push back error code
			}
		}
	}

	if(tchain.elims_done) return 1; ///

	if(rbn_it2) { // some case 2 to apply
		for(int i = 0; i < rbn_it2; i++) {
			BFTAG * ptg = & rbn_elimst2[i];
			USHORT ttt[] = {rbn_t2[i], rbn_t2[i] ^ 1};
			if(options.ot && rbase>100){
				EE.Enl("call rat nest a ~a for");
				zpln.ImageTag(ttt[0]);EE.Esp();
				zpln.ImageTag(ttt[1]);EE.Enl();
				Image (rbn_elimst2[i],"potential",0);
				EE.Enl();
			}

			for(int j = 3; j < col; j++) {
				if(ptg->On(j)) {   

					Rating_Nested( ttt, 2 ,j);
                if(stop_rating) return 1;// push back error code

				}
			}
		}
	}

	if(rbn_itch) { // some case 3 through sets to apply
		for(int i = 0; i < rbn_itch; i++) {

			if(tchain.elims_done) return 1; ///


			BFTAG * ptg = & rbn_tchte[i];
			SET chx = zcx.zc[rbn_tch[i]];
			if(0) {
				EE.E("action itch pour i =");
				EE.E(i);
				EE.E(" set=");
				EE.E(rbn_tch[i]);
				chx.Image(this,&EE);
				EE.Enl(); 
				Image((*ptg),"for targets ",0);
			}
			int n = 0, nni = chx.ncd; 
			USHORT ttt[20];
			for(int j = 0; j < nni; j++)
				ttt[j] = chx.tcd[j] << 1;
			USHORT p[640],np;
			ptg->String(p,np);
			for(int jx = 0; jx <np; jx++) {
			    int j=p[jx];
			// if(rbn_tch[i]==1) opdiag=1; else opdiag=0; ///
				Rating_Nested(ttt, nni, j);
                if(stop_rating) return 1;// push back error code

				
			}
		}
	}
	return Rating_end(200);
}

/* partial process for Rating_baseNest
   done either using d_nested2 or d_nested
   if d_nested gives results, take it
*/

void PUZZLE::Rbn_Elims( BFTAG * tsquare,int nn){
	// cases 1 and 2,{ a=>x a=> ~x } { x => ~a and ~x=> ~a}
	// case 2 includes the so called cell or region eliminations for bi values
	rbn_d_nested=tsquare;
	rbn_elims1.SetAll_0();
	rbn_elims2=rbn_elims3=rbn_elims1;
	rbn_it2=0;
	for(int i = 2; i < col; i += 2) {
		BFTAG tw (tsquare[i]); // full expansion for the tag
		BFTAG tw1 (tw);
		tw1 &= tw.Inverse();
		BFTAG tw2 ( tw);
		tw2 &= tsquare[i ^ 1];
		tw2 = tw2.FalseState();
		tw1 = tw1.TrueState();
		if(tw2.IsNotEmpty()) {  // case 2
			rbn_elimst2[rbn_it2] = tw2;
			rbn_t2[rbn_it2++] = i;
			rbn_elims2 |= tw2;
		}
		if(tw1.IsNotEmpty()) {  // case 1
			rbn_elims1.Set(i);
		}
	}

	// case 3, is there a set killing "a"
	rbn_itch = 0;
	for(int ie = 1; ie < zcx.izc; ie++) {
		SET chx = zcx.zc[ie];
		int n = 0, nni = chx.ncd; 
		if(chx.type - SET_base)
			continue;
		BFTAG bfw(BFTAG::InitFalseState);
		for(int i = 0; i < nni; i++){
			if(0)  
				Image(tsquare[chx.tcd[i] << 1],"dpn",chx.tcd[i] << 1);
			bfw &= tsquare[chx.tcd[i] << 1];
		    if(0) { 
 				chx.Image(this,&EE);
				Image(bfw,"communs",0);
			}
		}

		if(bfw.IsNotEmpty()) {
			if(0 && options.ot && rbase > 100) {
                chx.Image(this,&EE); 
			    Image(bfw," actif for ",0);
			}
			rbn_tchte[rbn_itch] = bfw;
			rbn_tch[rbn_itch++] = ie;
			rbn_elims3 |= bfw;
		}
	}// end case 3

	rbn_elimt = rbn_elims1.Inverse();
	rbn_elimt |= rbn_elims2;
	rbn_elimt |= rbn_elims3;
	if(options.ot && rbn_elimt.IsNotEmpty() ){ 
		EE.E("summary of first phase call nn= ");EE.Enl( nn);
		EE.E("itch ="); EE.E(rbn_itch);
		EE.E(" it2 ="); EE.Enl(rbn_it2);
		//Image(rbn_elimt,"elimt potential", 0);
		Image(rbn_elims1.Inverse(),"elims1 potential", 0);
		Image(rbn_elims2,"elims2 potential", 0);
		Image(rbn_elims3,"elims3 potential", 0);
	}
}

/* main routine to process chains
   entry at a specific basic value for min rating
   process is cancelled as soon as a solution is found with a shorter rating
   all chain with the minimum rating  are stored
*/
// chain call 1=biv 2= cell_bivalue 4=nishio 8 dynamic 16=multi_chain

void PUZZLE::Chaining(int opt, int level, int base) {
	TaggingInit();
	tchain.SetMaxLength(base);
	rbase=base;
	// long tta,ttc;// provisoire, pour test de temps
	int ir = 0;   
	if(options.ot) {
		EE.E("entree chaining opt=");
		EE.E(opt);
		EE.E(" base=");
		EE.E( base);
		EE.E(" level=");
		EE.Enl( level);
	}

	if((opt & 3) == 2) {  // "Y" mode slightly different
		zpln.CellStrongLinks();
		zpln.RegionLinks(0);
	}
	else {
		if(opt & 2)
			zpln.CellLinks(); 
		zpln.RegionLinks(opt & 1);
	}

	if(opt & 4) { // dynamic mode 
		if(opt & 2)
			zpln.GenCellsSets(); 
		zpln.GenRegionSets();  
	}

	switch(base) {
		case 65:
		case 70:    // same process for X;Y;XY loops and chains
			zcf.Aic_Cycle(opt & 3);     // several tags
			break;
		case 75: //  once derived weak links from directs
			zcf.hdp_base=zcf.hdp_dynamic=zcf.h.dp; // save basic weak links
	        zcf.ExpandShort(3);
            zcf.DeriveCycle(3, 9, 0,3);
	        ChainPlus();
            if(tchain.IsOK(77)) break;   // most often very short

			// must have here a step with full expansion
			zcf.h.d.ExpandAll((*this), zcf.h.dp);
			if(tchain.IsOK(78))      //filter  short paths
				break;
 
			while(zcf.DeriveCycle(3, 9, 0))
      	    	;// and full expansion
	        ChainPlus();

			break;
	}
}

//========= end of a rating step, check if done or still to be searched

   /* next is the next rating to be searched.
      the search is over if a <= rating has been found.
      likely <= next+1 due to the minimum length adjustment 
      except for aligned triplet 
	   // if some immediate action done, answer yes imediatly)
	   // nothing to do for filters
	  
	  */

int PUZZLE::Rating_end(int next) {
	if(stop_rating) return 1;// push back error as soon as possible
	if(tchain.elims_done) 		return 1;
	if(!tchain.IsOK(next))
		return 0;
	Step((SolvingTechnique)tchain.rating);
	if(rating_ir)
		return 0; 
	return tchain.Clean();
}

void PUZZLE::TaggingInit() {
	zcf.Init();     // elementary weak links
	zcx.Init();     // sets (choices) in use
}

// chain call 1=biv 2= cell_bivalue 4=nishio 8dynamic 16=multi_chain
//==============================================
// this includes the search for x or y cycle
int PUZZLE::Rating_base_65() {
	Chaining(1,0,65);// x cycle  
	if(Rating_end(65))
		return 1;
	Chaining(2, 0, 65);// y cycle  
	return Rating_end(70);
} 

//=============================================
// this includes the search for xy cycle or chain
int PUZZLE::Rating_base_70() {
	Chaining(3, 0, 70); // xy  forcing chain or loop
	return Rating_end(75);
} 

//==============================================
// this does not includes the search for aligned triplet 
// only th search for nishio
int PUZZLE::Rating_base_75() {
	Chaining(1 + 4, 0, 75); 
	return Rating_end(200);
} 

//==============================================
// 80 is multi chains
// we enter now a new group of eliminations. all the work is done using
// all bi values, basic weak links, basic sets
int PUZZLE::Rating_base_80() {
	if(options.ot)
		EE.Enl("start rating base 8.0 multi chains");
	TaggingInit();
	zpln.CellLinks();
	zpln.RegionLinks(1);
	zpln.GenCellsSets();
	zpln.GenRegionSets();
	zcf.hdp_base = zcf.hdp_dynamic = zcf.h.dp; // save basic weak links
	tchain.SetMaxLength(80);
	rbase = 80;
	zcf.ExpandAll();
	if(zcx.Interdit_Base80())
		return 1;
	return Rating_end(200);
}

/* 85 new process
   expand completely the tags, but try to do it to catch the shortest
   first steps are not standard
   finish with standard expansion
   then look for elimintaions
   */
//==============================================
// 85 is DynamicForcingChain

int PUZZLE::Rating_base_85() {
	if(options.ot)
		EE.Enl("start rating base 8.5 dynamic forcing chain");
	tchain.SetMaxLength(85);
	rbase=85;
	zcf.h.dp=zcf.hdp_base; // restore the  basic weak links
//	zcx.DeriveDirect();  // start with only equivalence to pointing claiming
    zcf.ExpandShort(5);
	zcf.DeriveCycle(3, 4, 0,8); // one cycle short sets
	zcf.DeriveCycle(3, 4, 0,8); // one cycle short sets
	ChainPlus();
	if(tchain.IsOK(88))      //filter  short paths
        return Rating_end(200);
	zcf.DeriveCycle(3, 5, 0,8); // one cycle short sets
	zcf.DeriveCycle(3, 5, 0,8); // one more cycle 
	ChainPlus();
	if(tchain.IsOK(90))      //filter  short paths
        return Rating_end(200);
	// must have here a step with full expansion
    zcf.h.d.ExpandAll((*this), zcf.h.dp);
	if(tchain.IsOK(92))      //filter  short paths
        return Rating_end(200);

	while(zcf.DeriveCycle(3, 9, 0))
		;// and full expansion

	ChainPlus();
	return Rating_end(200);
}


/* as in dynamic forcing chains,
   first a full controlled expansion
   then the search for the shortest
   */
//==============================================
// 90 is DynamicForcingChainPlus
// all "events"  claiming, pointing, pair, hidden pair, XWing
// follows an empty step 85
// consider each false candidate as start
// search for new bi values. If none, skip it
// look for new false thru basic sets
int PUZZLE::Rating_base_90() {
	if(options.ot)
		EE.Enl("start rating base 9.0 dynamic forcing chains plus");
		
	tchain.SetMaxLength(90);
	rbase = 90;
	zcf.h.dp = zcf.hdp_base; // restore the index in zcf  
	tevent.LoadAll();

	zcf.hdp_dynamic = zcf.h.dp; // store it for next steps

	zcf.h.d.ExpandShort(*this, zcf.h.dp, 2);
	zcf.DeriveCycle(3, 4, 7, 2); // one cycle;
	ChainPlus();
	if(tchain.IsOK(92))      //filter  short paths
        return Rating_end(200);
	zcf.DeriveCycle(3, 7, 7, 5); // one cycle;  
	ChainPlus();
	if(tchain.IsOK(94))      //filter  short paths
        return Rating_end(200);
	if(options.ot)
		EE.Enl("rating  dynamic forcing chains plus empty after 2 cycles");
	// must have here a step with full expansion
    zcf.h.d.ExpandAll((*this), zcf.h.dp);
	if(tchain.IsOK(96))      //filter  short paths
        return Rating_end(200);

	while(zcf.DeriveCycle(3, 9, 10)) // no limit in set size
		;
	ChainPlus();

	return Rating_end(200);
}

/* looking for elimination in dynamic mode
   in SE, such eliminations are always worked out of 2 chains
   analyzed from the left to the right.

   That process works in the same way, so elimination is one of the followings

   a-> b  and a -> ~b  (the classical  form a->~a is included here)
   x-> ~a and ~x -> ~a   
   region or cell multi chain  a -> {~x ~y ~z);

   the process has been reshaped to fit with search at higher levels
   */

void PUZZLE::ChainPlus() {
	int godirect = ((rbase + 8) <= ermax);
	BFTAG *t = zcf.h.d.t, *tp = zcf.h.dp.t; 
	if(0 && options.ot && rbase==85 && couprem<2) {
		EE.Enl("\n\nchain plus entry" );
		Image(zcf.h.d	);
	}           


	for(int i = 2; i < col; i += 2) {
		int icand = i >> 1;
		if(dynamic_form1.Off(i)){  
		   BFTAG zw1 = t[i];
		   zw1 &= (t[i].Inverse()).TrueState();
		    if(zw1.IsNotEmpty()) { // this is a a-> b  and a -> ~b
		        dynamic_form1.Set(i); 
				if(1 && options.ot) {
				   EE.E("\n\nfound active a->x and a->~x  a=" );
				   zpln.ImageTag(i);
				   EE.Enl();
	    		   //Image(zw1," elims",i);
			    }           
           if(godirect) 
			   tchain.ClearImmediate(icand);
		   else
			   GoNestedCase1(icand); 
			}
		}

		BFTAG zw2 = t[i];
		zw2 &= t[i ^ 1];
		zw2 = zw2.FalseState();
		zw2-=dynamic_form2[icand]; // less already processed in dynamic search

		// if we start from a bi value, SE does not say
		// ~x but goes immediatly to the bi-value  saving one step.
		// 
		if(zw2.IsNotEmpty()) { // this is x-> ~a and ~x -> ~a
			dynamic_form2[icand] |= zw2;  
			if(1 && options.ot) {
				EE.E("\n\nfound active x->~a and ~x->~a");
				Image(zw2, "elims", i);
			}
			if(godirect) {
				for(int j = 3; j < col; j += 2)
					if(zw2.On(j))
						tchain.ClearImmediate(j >> 1);
			}
			else {
				USHORT ttt[] = {i, i ^ 1};
				zcf.h.dp.Shrink(zw2, i);
				Image(zw2,"elims solde", i);
				for(int j = 3; j < col; j += 2)
					if(zw2.On(j))	
						Rating_Nested(ttt, 2, j);
			}
		}// end if zw2
	} // end for i

	if(rbase < 85) // || tchain.elims_done)  cancelled to be closer to serate
		return;
	if(1 && options.ot) {
		  EE.Enl("\n\ncheck also sets" );
	}
	godirect = ((rbase + 10) <= ermax);
	// now check multichains in a similar way but not for nishio
	int ie;
	for(ie = 1; ie < zcx.izc; ie++) {
		const SET &chx = zcx.zc[ie];
		if(chx.type - SET_base)
			break; // only base sets can be used
		int n = chx.ncd;
		USHORT *tcd = chx.tcd,
			   ttt[20];
		BFTAG tbt, bfset;
		tbt.SetAll_1();
		tbt -= dynamic_sets[ie]; // already seen in dynamic mode
		for(int i = 0; i < n; i++){
			ttt[i] = (tcd[i] << 1);
			bfset.Set(ttt[i] ^ 1);
		}
		for(int i = 0; i < n; i++) {
			BFTAG tw = t[tcd[i] << 1];
			tw -= bfset;
			tbt &= tw;
		}
		if(tbt.IsEmpty())
			continue;

		dynamic_sets[ie] |= tbt;  

		if(1 && options.ot) {
		  EE.E("set active" );chx.Image(this,&EE);
		  Image(tbt,"for",0); EE.Enl( );
		}


        for(int j = 3; j < col; j += 2) if(tbt.On(j))	
			if(godirect)  
				tchain.ClearImmediate(j>>1);
			else
				Rating_Nested(ttt,n,j);

	}// end for ie
}

//! Search for aligned pair or aligned triplet
/**
	That proces is very close to SE's one.
	Find all cells with 2 or 3 candidates and 
	collect all cells seen by one of them (potential base cells)
	First choose 2 cells (in the same band)of the potential base cells.
*/
int PUZZLE::AlignedTripletN() {
	int debuga = 0;
	static const int combi32[3][2] = {{0,1},{0,2},{1,2}};
	BF81 z23,	// set of cells that have 2 or 3 candidates (could be excluding cells)
		zbase;	// set of cells that are visible by one potential excluding cells
	for(int i = 0 ;i < 81; i++) {
		CELL_VAR p = T81t[i].v;
		if(p.ncand < 2 || p.ncand > 3)
			continue;
		z23.Set(i); 
		zbase |= cellsFixedData[i].z;
	}
	zbase &= zactif; // possible base cells reduced to cells that have no value
	// loop on any combination of 2 potential base cells
	for(int i1 = 0; i1 < 80; i1++) {
		if(!zbase.On(i1))
			continue;
		// if((z23 & cellsFixedData[i1].z).Count()>=3) // optimisation non concluante 
		// il faudrait optimiser Count (qui balaye le tableau de bits !)
		for(int i2 = i1 + 1; i2 < 81; i2++) {
			if(!zbase.On(i2))
				continue;
			// if((z23 & cellsFixedData[i2].z).Count()>=3) // optimisation non concluante 
			// il faudrait optimiser Count
			BF81 basei(i1, i2); // first 2 base cells
			const t_128 *zi1 = &cellsFixedData[i1].z;
			const t_128 *zi2 = &cellsFixedData[i2].z;
			BF81 zi1andi2 = (*zi1);
			zi1andi2 &= (*zi2);
			if(zi1andi2.Count() < 2)
				continue; // must be same band
			// we need to add an other base cell + 2 excludig cells that are visible
			// by the 3 base cells => the 3 base cells must share a same band
			// this condition is not used by serate
			//BF81 zi1ori2 = ((*zi1) | (*zi2)) - basei; // excluding already there
			BF81 zi1ori2 = *zi1;
			zi1ori2 |= *zi2;
			zi1ori2 -= basei; // excluding already there
			BF81 z23ori1i2 = z23 & zi1ori2; // seen by at least one
			// z23ori1i2 is the "twinarea of serate"
			if(z23ori1i2.IsEmpty())
				continue; // need a third cell as base cell
			for(int i3 = 0; i3 < 81; i3++) { // try all possible third cell
				if(!z23ori1i2.On(i3))
					continue;
				if(i1 == 39 && i2 == 53 && i3 == 40) //TODO
					debuga = 1;
				BF81 basei3 = basei;
				basei3.Set(i3);	// set of 3 base cells
				BF81 z23f = (zi1andi2 & cellsFixedData[i3].z & z23) - basei3; // TODO verify but ' - basei3' is de trop
				if(z23f.Count() < 2)
					continue; // we need at least 2 potential excluding cells
				int z23fIndex[81];
				int z23fSize = z23f.String(z23fIndex);

				// TODO modify 

				// loop on permutation of potential candidate for the 3 base cells
				int iCell[3] = {i1, i2, i3};		// index of base cells 0-80
				int ch[3];				// candidate for the 3 base cells 8-0
				BF16 cands[3] = {T81t[i1].v.cand, T81t[i2].v.cand, T81t[i3].v.cand};	// set of candidates
				BF16 allowed[3];
				for(int k1 = 0; k1 < 3; k1++)
					allowed[k1].SetAll_0();
				// test all combination of candidates
				for(ch[0] = 0; ch[0] < 9; ch[0]++) {
					if(!cands[0].On(ch[0]))
						continue;
					for(ch[1] = 0; ch[1] < 9; ch[1]++) {
						if(!cands[1].On(ch[1]))
							continue;
						for(ch[2] = 0; ch[2] < 9; ch[2]++) {
							if(!cands[2].On(ch[2]))
								continue;
							bool allow = true;
							// verify if this combination is forbidden 
							// 2 base cells that are visible may not have the same candidate
							for(int combiIndex = 0; combiIndex < 3; combiIndex++) {
								int ch1 = ch[combi32[combiIndex][0]];
								int ch2 = ch[combi32[combiIndex][1]];
								if (ch1 == ch2) {
									int ic1 = iCell[combi32[combiIndex][0]];
									int ic2 = iCell[combi32[combiIndex][1]];
									if(T81t[ic1].f->ObjCommun(T81t[ic2].f)) {
										allow = false;
										break;
									}
								}
							}
							if(!allow)
								continue;
							// verify if the combination is forbidden by an excluding cell
							// the potential candidate clear all possibliites for the exlcuding cell
							// loop on potential excluding cell
							//for(int iec = 0; iec < 81; iec++) { //v0 by GP
							//	if(!z23f.On(iec))
							//		continue;
							//	BF16 cec = T81t[iec].v.cand;
							//	for(int ich = 0; ich < 3; ich++)
							//		cec.Clear(ch[ich]);
							//	if(cec.isEmpty()) {
							//		allow = false;
							//		break;
							//	}
							//}
							for(int ecIndex = 0; ecIndex < z23fSize; ecIndex++) { //v1 by MD
								BF16 cec = T81t[z23fIndex[ecIndex]].v.cand;
								for(int ich = 0; ich < 3; ich++)
									cec.Clear(ch[ich]);
								if(cec.isEmpty()) {
									allow = false;
									break;
								}
							}
							if(!allow)
								continue;
							// combination allowed => 3 candidates are allowed
							for(int cellIndex = 0; cellIndex < 3; cellIndex++)
								allowed[cellIndex].Set(ch[cellIndex]);
						} // end ch 3
					} // end ch 2
				} // end ch 1
				// verify for each potential candidates if there are no allowed possibility
				for(int ice2 = 0; ice2 < 3; ice2++) { // loop on base cells
					if(cands[ice2] == allowed[ice2])
						continue; // optimisation
					for(int icand = 0; icand < 9; icand++) { // loop on candidates
						if(!cands[ice2].On(icand))
							continue;
						if(!allowed[ice2].Off(icand))
							continue;
						// we have found one "aligned triplet exclusion"
						T81t[iCell[ice2]].Change(icand,this); 
						if(options.ot) {
							EE.E(" aligned triplet exclusion for ");
							EE.E(icand+1);
							EE.E(cellsFixedData[iCell[ice2]].pt);
							EE.E(" using base cells ");
							EE.E(cellsFixedData[i1].pt);
							EE.E(" , ");
							EE.E(cellsFixedData[i2].pt);
							EE.E(" and ");
							EE.E(cellsFixedData[i3].pt);
							EE.E(" ; ");
							EE.Enl();
							EE.E(" using excluding cells ...");
							// TODO add set of excluding cells
							EE.Enl();
						}
						return 1;
					} // end icand
				} // end  ice2
			} // end for i3
		} // end for i2
	} // end for i1
	debuga = 1; //TODO
	return 0;
}

//similar process for aligned pair with 2 base cells only

//! Search for aligned pairs
int PUZZLE::AlignedPairN() {
	BF81 z2, zbase;
	// look for cells that have 2 candidates
	for(int i81 = 0; i81 < 81; i81++) {
		CELL_VAR p = T81t[i81].v;	// candidates of the cell
		if(p.ncand - 2)
			continue; // only cell with 2 candidates
		z2.Set(i81);			// put cell to set
		zbase |= cellsFixedData[i81].z;	// or of cells controled by these cells
	}
	zbase &= zactif;		// keep only cells that have not yet a value
	for(int i1 = 0; i1 < 80; i1++) {
		if(!zbase.On(i1))
			continue;
		for(int i2 = i1 + 1; i2 < 81; i2++) {
			if(!zbase.On(i2))
				continue;
			// a couple of cells  that see at minimum one bivalue cell
			BF81 basei(i1, i2);
			const t_128 *zi1 = &cellsFixedData[i1].z; // influence zone of first cell
			const t_128 *zi2 = &cellsFixedData[i2].z; // influence zone of second cell
			BF81 z2f = *zi1;
			z2f &= *zi2;
			z2f &= z2; // set of cell with 2 cand in both influence zones
			if(z2f.Count() < 2)
				continue;  
			// z2f is the set of excluding cells of serate
			// test combination of candidates for cell i1 and i2
			int ch1 = 0;					// candidate for cell i1
			BF16 cands1 = T81t[i1].v.cand;	// set of candidates
			int ch2 = 0;					// candidate for cell i2
			BF16 cands2 = T81t[i2].v.cand;	// set of candidates
			int excl1[9];	
			int excl2[9];
			for(int k = 0; k < 9; k++) {
				excl1[k] = excl2[k] = 0;
			}
			// test all combination of candidates
			for(ch1 = 0; ch1 < 9; ch1++) {
				if(!cands1.On(ch1))
					continue;
				for(ch2 = 0; ch2 < 9; ch2++) {
					if(!cands2.On(ch2))
						continue;
					// look if visible cells and same cand => excluding
					if(ch1 == ch2) {
						if(T81t[i1].f->ObjCommun(T81t[i2].f)) {
							excl1[ch1]++;
							excl2[ch2]++;
							continue;
						}
					}
					else {
						// look if there is an exclusion cell
						for(int i = 0; i < 81; i++) {
							if(!z2f.On(i))
								continue;
							if(T81t[i].v.cand.On(ch1) && T81t[i].v.cand.On(ch2)) {
								excl1[ch1]++;
								excl2[ch2]++;
								break;
							}
						}
					}
				}
			}
			// verify if there is an excluded candidate
			int n1 = T81t[i2].v.ncand;
			int n2 = T81t[i1].v.ncand;
			int cell8 = -1;
			int ch = -1;
			for(ch1 = 0; ch1 < 9; ch1++) {
				if(!cands1.On(ch1))
					continue;
				if(excl1[ch1] == n1) {
					cell8 = i1;
					ch=ch1;
					break;
				}
			}
			if(cell8 < 0) {
				for(ch2 = 0; ch2 < 9; ch2++) {
					if(!cands2.On(ch2))
						continue;
					if(excl2[ch2] == n2) {
						cell8 = i2;
						ch = ch2;
						break;
					}
				}
			}
			if(cell8 >= 0) { // we have found one pair exclusion
				T81t[cell8].Change(ch,this); // erase candidate
				if(options.ot) {
					EE.E(" aligned pair exclusion for ");
					EE.E(ch+1);
					EE.E(cellsFixedData[cell8].pt);
					EE.E(" using base cells ");
					EE.E(cellsFixedData[i1].pt);
					EE.E(" and ");
					EE.E(cellsFixedData[i2].pt);
					EE.E(" ; ");
					EE.Enl();
					EE.E(" using excluding cells ");
					for(int i = 0; i < 81; i++) {
						if(z2f.On(i)) {
							EE.E(cellsFixedData[i].pt);
							EE.E(" ; ");
						}
					}
					EE.Enl();
				}
				return 1;
			}// ich
		} // i2
	} // i1
	return 0;
}

int PUZZLE::Traite(char * ze) {
	stop_rating = 0;
	cycle = assigned = c_ret = 0;
	ermax = epmax = edmax = 0;

	// final location for the normalized puzzle in a global variable
	for(int i = 0; i < 81; i++) //get normalised puzzle in puz
		if(ze[i] - '.')
			gg.pg[i] = ze[i];
		else gg.pg[i] = '0';

	// check if the puzzle is correct 
	//(no duplicate givens in a house)
	// one solution and only one
	char puz_zero_based[81];
	char solutions[2][81];
	for(int i = 0; i < 81; i++) {
		puz_zero_based[i] = (ze[i] >= '1' && ze[i] <= '9') ? ze[i] - '0' : 0;
	}
	if(1 != fsss(puz_zero_based, 2, solutions[0])) { //zero or more than one solutions
        edmax=1;
		return 0;
	}
	for(int i = 0; i < 81; i++) {
		solution[i] = solutions[0][i] + '0';
	}

	/* the solution is stored in an appropriate form 
	 * 9 81 bitfield indicating for each digit the positions
	 * to check later the validity of eliminations
	 * this is a debugging control
	 */
	//for(int i=0;i<9;i++) 
	//	csol[i].SetAll_0();
	//for(int i=0;i<81;i++) 
	//	csol[solution[i]-'1'].Set(i);


	//================== assign clues to start
	cInit(1);
	PKInit();
	//tdebut = GetTimeMillis();
	//long told = tdebut;
	T81->init();  //initial candidats au lieu de Traite_Init(); 
	for(int i = 0; i < 81; i++) { // first assignment from clues
		int w = gg.pg[i];
		if(w > '0' && w <= '9')
			T81->Fixer(w - '1', i, 2);
	}
	cReport(); // and preparing T81 from PM per digit

	EE.Enl(); // new line in case test is done

	//===========================================================
	while (cycle++ < 150) {
		if(cycle > 148) {
			Seterr(7);
			break;
		} // 
		if(stop_rating) {
			Seterr(5);
			break;
		} 

		int ir_ed = is_ed_ep();
		if(ir_ed > 2)
			return ir_ed;//split filter
		if(ir_ed)
			return 0; // pure ed ep filter
		if(!Recale()) {
			Seterr(6);
			break;
		}
		if(!gg.NBlancs())
			break; // finished
		// processing 1.0 to <6.2
		int ir_a = Traite_a();    

		if(!ir_a)
			return rating_ir;
		else if(ir_a < 2)
			continue;

		//  start of processes using the tagging infrastructure.
		// the table of candidates is loaded once per cycle

		zpln.Init();  // table of candidates  
		if(stop_rating)
			break; // in case we would exceed the TCAND limit
		//=========================
		tchain.NewCycle();
		Step(AlignedPairExclusion);
		if(rating_ir > 1)
			return rating_ir;
		else if(rating_ir)
			continue;
		if(AlignedPairN()) {
			SetEr();
			continue;
		}  //6.2

		if(ermax < 74) { // skip that if high ER already reached
			Step(AIC_X_cycle);
			if(rating_ir > 1)
				return rating_ir;
			else if(rating_ir)
				continue;
			if(Rating_base_65()) {
				SetEr();
				continue;
			}  //6.5 and 6.6 
			Step(AIC_XY);
			if(rating_ir > 1)
				return rating_ir;
			else if(rating_ir)
				continue;
			if(Rating_base_70()) {
				SetEr();
				continue;
			}  //70
		}
		else if(ermax < 77) { // just skip 6.5 6.6 as a first step
			if(options.ot)
				EE.Enl("go direct to XY"); 
			Step(AIC_XY);
			if(rating_ir > 1)
				return rating_ir;
			else if(rating_ir)
				continue;
			if(Rating_base_70()) {
				SetEr();
				continue;
			}  //70
		}		
		else {
			if(options.ot)
				EE.Enl("gofast");
			if(zcf.Fast_Aic_Chain())
				continue;
		} 

		Step(AlignedTripletExclusion);
		if(rating_ir>1)
			return rating_ir;
		else if(rating_ir)
			continue;
		if(AlignedTripletN()) {
			SetEr();
			continue;
		}  //7.5  

		// at that point, we set to nill dynamic processing done
		// this is done once for all the cycle
		dynamic_form1.SetAll_0();
		for(int i=0;i<320;i++){
			dynamic_form2[i].SetAll_0();
			dynamic_sets[i].SetAll_0();
		}

		if(Rating_base_75()) {  
			SetEr();
			continue;
		}  //7.5
		if(options.oexclude - 1) {
			Step(MultipleForcingChain);
			if(rating_ir > 1)
				return rating_ir;
			else if(rating_ir)
				continue;
			if(Rating_base_80()) {
				SetEr();
				continue;
			}  //8.0

			if(options.oexclude - 2) {	 	
				Step(DynamicForcingChain);
				if(rating_ir>1)
					return rating_ir;
				else if(rating_ir)
					continue;
				if(Rating_base_85()) {
					SetEr();
					continue;
				}  //8.5

				if(options.oexclude - 3) {	
					Step(DynamicForcingChainPlus);
					if(rating_ir > 1)
						return rating_ir;
					else if(rating_ir)
						continue;
					if(Rating_base_90()) {
						SetEr();
						continue;
					}  //9.0

					if(options.oexclude - 4) {
						InitNested(); 
						Step(NestedForcingChain);
						if(rating_ir>1)
							return rating_ir;
						else if(rating_ir)
							continue;
						if(Rating_baseNest(95, options.oq)) {
							SetEr();
							continue;
						}  //9.5

						if(options.oexclude - 5) {	
							Step(NestedLevel3);
							if(rating_ir>1)
								return rating_ir;
							else if(rating_ir)
								continue;
							if(Rating_baseNest(100, options.oq)) {
								SetEr();
								continue;
							}  //100
							if(options.oexclude - 6) {
								Step(NestedLevel4);
								if(rating_ir > 1)
									return rating_ir;
								else if(rating_ir)
									continue;
								if(Rating_baseNest(105, options.oq)) {
									SetEr();
									continue;
								}  //105
							}// end level4
						}// end level 3 and more
					} // end level 2 and more
				} // end if >2
			} // end if >1
		} // end if oexclude
		if(Rating_end(200)) {
			SetEr();
			continue;
		} // clean the file
		if(options.ot)
			T81->Candidats();
		stop_rating=2;
		break;
	}     
	//=========================================================================	 
	EE.E("fin traitement stop_rating=");
	EE.Enl(stop_rating );
	gg.Image(&EE, "fin");
	return stop_rating;
}

int PUZZLE::Traite_a() {
	if (Directs ()) {
		Step(LastCell);
		if(rating_ir > 1)
			return 0;
		else if(rating_ir)
			return 1;
		if(FaitDirects(LastCell)) {
			SetEr();
			return 1;
		}   //1.0

		Step(SingleBox);
		if(rating_ir > 1)
			return 0;
		else if(rating_ir)
			return 1;
		if(FaitDirects(SingleBox)) {
			SetEr();
			return 1;
		}  //1.2

		Step(Single_R_C);
		if(rating_ir > 1)
			return 0;
		else if(rating_ir)
			return 1;
		if(FaitDirects(Single_R_C)) {
			SetEr();
			return 1;
		}  //1.5
	}
	Actifs(); // update of active cells must be done from here

	Step(Single_after_Locked);
	if(rating_ir > 1)
		return 0;
	else if(rating_ir)
		return 1;
    if(TraiteLocked(Single_after_Locked)) {
		SetEr();
		return 1;
	}  //1.7

	Step(HiddenPair_single);
	if(rating_ir > 1)
		return 0;
	else if(rating_ir)
		return 1;
    if(yt.Tiroirs(2,1,1)) {
		SetEr();
		return 1;
	}  //2.0

	if(Directs()) {
	  Step(NakedSingle);
	  if(rating_ir > 1)
		  return 0;
	  else if(rating_ir)
		  return 1;
	   if(FaitDirects(NakedSingle)) {
		   SetEr();
		   return 1;
	   }
	}  //2.3

    Step(HiddenTriplet_single);
	if(rating_ir > 1)
		return 0;
	else if(rating_ir)
		return 1;
    if(yt.Tiroirs(3,1,1)) {
		SetEr();
		return 1;
	}  //2.5

	Step(Locked_box);
	if(rating_ir > 1)
		return 0;
	else if(rating_ir)
		return 1;
    if(TraiteLocked(Locked_box)) {
		SetEr();
		return 1;
	}  //2.6

	Step(Locked_RC);
	if(rating_ir > 1)
		return 0;
	else if(rating_ir)
		return 1;
    if(TraiteLocked(Locked_RC)) {
		SetEr();
		return 1;
	}  //2.8

	Step(NakedPair);
	if(rating_ir > 1)
		return 0;
	else if(rating_ir)
		return 1;
    if(yt.Tiroirs(2,0,0)) {
		SetEr();
		return 1;
	}  //3.0

	Step(XWing);
	if(rating_ir > 1)
		return 0;
	else if(rating_ir)
		return 1;
    if(yt.XW(2)) {
		SetEr();
		return 1;
	}  //3.2

	Step(HiddenPair);
	if(rating_ir > 1)return 0;
	else if(rating_ir)
		return 1;
    if(yt.Tiroirs(2,1,0)) {
		SetEr();
		return 1;
	}  //3.4

	Step(Naked_triplet);
	if(rating_ir > 1)
		return 0;
	else if(rating_ir)
		return 1;
    if(yt.Tiroirs(3,0,0)) {
		SetEr();
		return 1;
	}  //3.6

	Step(swordfish);
	if(rating_ir > 1)
		return 0;
	else if(rating_ir)
		return 1;
    if(yt.XW(3)) {
		SetEr();
		return 1;
	}  //3.8

	Step(HiddenTriplet);
	if(rating_ir > 1)
		return 0;
	else if(rating_ir)
		return 1;
    if(yt.Tiroirs(3,1,0)) {
		SetEr();
		return 1;
	}  //4.0

    if(options.ot)
		T81->Candidats();

    Copie_T_c(); // to be done now copie for UR same rating
    zpaires.CreerTable(T81t);  

	Step(XYWing);
	if(rating_ir > 1)
		return 0;
	else if(rating_ir)
		return 1;
    if(zpaires.XYWing()) {
		SetEr();
		return 1;
	}  //4.2

	Step(XYZWing);
	if(rating_ir > 1)
		return 0;
	else if(rating_ir)
		return 1;
    if(zpaires.XYZWing()) {
		SetEr();
		return 1;
	}  //4.4


	Step(UniqueRect1);
	if(rating_ir > 1)
		return 0;
	else if(rating_ir)
		return 1;
	if(T81->RIN()) {
		SetEr();
		return 1;
	}  //4.5

	Step(UniqueRect2);
	if(rating_ir > 1)
		return 0;
	else if(rating_ir)
		return 1;
    if(urt.Traite(46)) {
		SetEr();
		return 1;
	}  //4.6 t
    if(zpaires.UL()) {
		SetEr();
		return 1;
	}    

	Step(UniqueRect3);
	if(rating_ir > 1)
		return 0;
	else if(rating_ir)
		return 1;
	 if(urt.Traite(47)) {
		 SetEr();
		 return 1;
	 } //4.7
	 if(tult.Traite(47)) {
		 SetEr();
		 return 1;
	 } 

	Step(UniqueLoop1);
	if(rating_ir > 1)
		return 0;
	else if(rating_ir)
		return 1;
	if(urt.Traite(48)) {
		SetEr();
		return 1;
	} //4.7
	if(tult.Traite(48)) {
		SetEr();
		return 1;
	}  //4.8
	                  	
	Step(UniqueLoop2);
	if(rating_ir > 1)
		return 0;
	else if(rating_ir)
		return 1;
	if(tult.Traite(49)) {
		SetEr();
		return 1;
	}  //4.9

	Step(NakedQuad);
	if(rating_ir > 1)
		return 0;
	else if(rating_ir)
		return 1;
 	if(tult.Traite(50)) {
		SetEr();
		return 1;
	}  //4.9
    if(yt.Tiroirs(4,0,0)) {
		SetEr();
		return 1;
	}  //5.0

	Step(UniqueLoop3);
	if(rating_ir > 1)
		return 0;
	else if(rating_ir)
		return 1;
	if(tult.Traite(51)) {
		SetEr();
		return 1;
	}  //5.1
 
	Step(Jellyfish);
	if(rating_ir > 1)
		return 0;
	else if(rating_ir)
		return 1;
	if(tult.Traite(52)) {
		SetEr();
		return 1;
	}  //5.2
    if(yt.XW(4)) {
		SetEr();
		return 1;
	}  //5.2

	Step(HiddenQuad);
	if(rating_ir > 1)
		return 0;
	else if(rating_ir)
		return 1;
    if(yt.Tiroirs(4,1,0)) {
		SetEr();
		return 1;
	}  //5.4

	Step(BUG);
	if(rating_ir > 1)
		return 0;
	else if(rating_ir)
		return 1;
    if(zpaires.BUG()) {
		SetEr();
		return 1;
	}  //5.6 a 5.8

	if(zpaires.aigun) {
		Step((SolvingTechnique)59);
		if(rating_ir > 1)
			return 0;
		else if(rating_ir)
			return 1;
		if(zpaires.Bug3a(59)) {
			SetEr();
			return 1;
		}  //5.9
		Step((SolvingTechnique)60);
		if(rating_ir > 1)
			return 0;
		else if(rating_ir)
			return 1;
		if(zpaires.Bug3a(60)) {
			SetEr();
			return 1;
		}  //6.0
		Step((SolvingTechnique)61);
		if(rating_ir > 1)
			return 0;
		else if(rating_ir)
			return 1;
		if(zpaires.Bug3a(61)) {
			SetEr();
			return 1;
		}  //6.1
	}
	return 2;
}

// part of PUZZLE class methos processing locked candidates in a box, row,col

void messlock(const OPSUDO &options, int obj, int obj2, int ch) {
	if(!options.ot)
		return;
	int it1 = obj2 / 9, it2 = obj / 9, ii1 = obj2 % 9, ii2 = obj % 9;
	char c1, c2;
	c1 = (it1 - 1) ? (char)(ii1 + '1') : lc[ii1];
	c2 = (it2 - 1) ? (char)(ii2 + '1') : lc[ii2];
	EE.E("-> ");
	EE.E(orig[it1]);
	EE.E(ii1 + 1);
	EE.E("  digit ");	
	EE.E(ch + 1);
	EE.E(" in ");
	EE.E(orig[it2]);
	EE.Enl(ii2 + 1);
}

/* rating difficulty linked to that include
    Single_after_Locked=17,		// locked in box  do only the fix
    Locked_box=26,				// locked in box  no fix
    Locked_RC=28,				// locked in row/col  no fix
  
*/

//<<<<<<<<<<<<<<<<<<<<
int PUZZLE::TraiteLocked(int rating) {
	if(rating == 26)
		return TraiteLocked2(18, 27); // box only no fix
	if(rating == 28)
		return TraiteLocked2(0, 18); // row col  no fix
	// now rating 1.7, process only boxes, 
	// must generate a new single in an attached box to elem2
	// clearing is done in the cell where the fix takes place
	int ir = 0;
	int ialt;
	for(int ich = 0; ich < 9; ich++) {
		BF81 wf = c[ich], wfel;
		for(int iel = 18; iel < 27; iel++) {
			wfel = wf & cellsInHouseBM[iel];
			if(wfel.IsEmpty())
				continue;
			if(DIVF::IsAutreObjet(wfel,iel,ialt)) {
				BF81 wa = wf & cellsInHouseBM[ialt];
				BF81 wex = wa ^ wfel;
				if(wex.IsNotEmpty()) {
					// the search for singles is done only in boxes
					// intersecting with ialt , so it must be a hidden single
					int ok = 0;
					BF81 ww;
					for(int i = 18; i < 27; i++) { // must be a box
						if(i != ialt) {
							ww = cellsInHouseBM[i];
							ww &= cellsInHouseBM[ialt];
							if(ww.IsNotEmpty()) {
								//ww = (divf.elz81[i] & c[ich]) - wex;
								ww = (c[ich] & cellsInHouseBM[i]) - wex;
								if(ww.Count() == 1) {
									ok = 1;
									break;
								}
							}
						}
					}
					if(ok) {  // clear others candidates in the cell to be fixed
						messlock(options, ialt, iel, ich);
						int i8 = ww.First();
						T81t[i8].Keep(ich,this);
						EE.Enl("lock assignment");
						return FaitGoA(i8, ich + '1', 4);
					} // immediate return after assign  
				}
			}
		}
	}
	return ir;
}

//<<<<<<<<<<<<<<<<<<<<
int PUZZLE::TraiteLocked2(int eld, int elf) {
	int ir = 0;
	int ialt;
	for(int ich = 0; ich < 9; ich++) {
		BF81 wf = c[ich], wfel;
		for(int iel = eld; iel < elf; iel++) {
			wfel = wf & cellsInHouseBM[iel];
			if(wfel.IsEmpty())
				continue;
			if(DIVF::IsAutreObjet(wfel, iel, ialt)) {
				BF81 wa = wf & cellsInHouseBM[ialt];
				BF81 wex = wa ^ wfel;
				if(wex.IsNotEmpty()) {
					messlock(options, ialt, iel, ich) ;
					T81->Clear(wex, ich);
					ir = 1;
					wf = c[ich] = wf ^ wex;
				}
			}
		}
	}
	return ir;
}

/* first step in the search for nested chains
   fill zcf.h_nest.d.t[tag]  with the full expansion
      in that mode
*/
///
void PUZZLE::GoNestedTag(USHORT tag) {
	opp = 0; // opdiag;  
	//if((rbase== 105) && (couprem==12) && (tag==8)	) opp=1; else opp=0;

	const BFTAG &tt (d_nested.t[tag]); 
    if(opp) {
			EE.E("start gonested tag ");
			EE.E(tag);
			zpln.ImageTag(tag);
			Image(tt,"depart nested",0); 
			EE.Enl(); 
		}	 	
	if(tt.IsEmpty())
		return;
	zcf.StartNestedOne();
	zcx.StartNestedOne();
	zcxb.StartNestedOne();
	to = zcf.h_one.dp.t; //forward and back tracking table 
	tcandgo.Init(); // intital storage for strong links and nested eliminations
	tstore.Init(); // start with an empty table of chains stored
	hdp_base_nested=dpn_old=zcf.hdp_base;

	//now go forward  till nothing or a contradiction

	for(int i = 0; i < 640; i++)
		tsets[i] = 0;
	npas = 0;
	steps[0] = tt; 
	tt.String(tx[0],itx[0]);
	steps[0].Set(tag); // add tag to known
	if(itx[0] >= 200)
		itx[0] = 200; // safety measure
	allsteps = cumsteps[0] = steps[0];  
	nested_aig = 1;
//	int maxpas = 30;  // should be enough in that mode
	int maxpas = 12;  // should be enough at level 4
	if(rbase<105) maxpas+=10; // more at level 3
	if(rbase<100) maxpas+=10; // and still more at level 2
	//--------------------- loop  forward
	while(nested_aig && npas++ <= maxpas) {
		if(opp) {
			EE.E("tag ");
			EE.E(tag);
			zpln.ImageTag(tag);
			EE.E(" npas= ");
			EE.Enl(npas); 
		}	 
		  // prepare the call to GoNestedWhileShort
		nested_aig = 0; 
		cum = &cumsteps[npas - 1];
		step = &steps[npas];
		ta = tx[npas - 1];
		ita = itx[npas - 1];

		if(opp) {
			Image((*cum),"cum debut",0);
		}
		GoNestedWhileShort(tag);                    // while cycle

		if(nested_aig){// something happenned
         
		  cumsteps[npas] = allsteps;   
		  (*step) = allsteps;
		  (*step)-=(*cum);  
		  (*step).String(tx[npas],itx[npas] );

		  if(opp) {
			Image((*step),"step", 0);
		  }
		   // store in d_nested2 if npas==2
		  if(npas<=2)
			  d_nested2.t[tag] = allsteps;
		  if(npas<=8)
			   d_nested8.t[tag] = allsteps;
		  d_nested.t[tag] = allsteps;
		   // stop the process if direct elimination appears
		  BFTAG wt(allsteps);
		  wt&=wt.Inverse();
		  if(wt.IsNotEmpty()){
			  return;
		  }
           
		}
	}// end while
	// note:the last one may be empty
}

/* while cycle  for GoNested first run, just find potential
   first apply new actions from direct implications and sets
   then check if new strong links appeared
   if new strong links, search for new indirect forcing chain
   if level, check anyway new multi chain eliminations 

*/
void PUZZLE::GoNestedWhileShort(USHORT tag) {
	USHORT aignl = 1;
	//const BFTAG &cum_here = *cum;
	BFTAG * tdpn = dpn.t,  // new set of direct links
	    * hdpb =hdp_base_nested.t; // to receive new strong links

	// look first for direct 
	for(int it = 0; it < ita; it++) {
		BFTAG x = to[ta[it]];
		if(x.substract(allsteps)) {
			if(opp)
				Image(x,"applied std" ,ta[it]);	   
			allsteps |= x; // and in the total 
			nested_aig = 1;
		}    
	}

	// check now sets

	for(int ie = 1; ie < zcx.izc; ie++) {
		const SET &chx = zcx.zc[ie];
		int n = 0, nni = chx.ncd, aig2 = 0, toff[10]; 
		switch (chx.type) {
		case SET_base:  // must be n-1 false or n false (then all is ok)
			// check if active 0 to 2 cand unknown
			{
				BFTAG bfw;
				for(int i = 0; i < nni; i++) { // max one free 
					USHORT cd = chx.tcd[i], j = cd << 1; // candidate in tag form
					if(cum->On(j)) {
						n = 3;
						break;
					}// set assigned
					if(cum->Off(j ^ 1)) {
						toff[n++] = j;
						if(n > 2)
							break;
					} 
					else
						bfw.Set(j ^ 1);
				}

				USHORT cd1 = toff[0], cd2 = toff[1]; 
				if(n == 2) {
					if(hdpb[cd1].Off(cd2 ^ 1) || 
					   hdpb[cd2].Off(cd1 ^ 1) ||
					   hdpb[cd1 ^ 1].Off(cd2) || 
					   hdpb[cd2 ^ 1].Off(cd1)) {
						hdpb[cd1].Set(cd2 ^ 1);
						hdpb[cd2].Set(cd1 ^ 1);
						hdpb[cd1 ^ 1].Set(cd2);
						hdpb[cd2 ^ 1].Set(cd1);
						aignl = 0;
						tcandgo.AddStrong(cd1 >> 1, cd2 >> 1, bfw, nni - 2);
                        nested_aig=1;  // must force new loop
					}
					break;
				}
				if(!n){  // nothing valid switch to a contradiction on any candidate
                    for(int i = 0; i < nni; i++) { 
                         USHORT cd = chx.tcd[i], j = cd << 1;
						 if(allsteps.Off(j)) {
						      allsteps.Set(j);
						      tsets[j] = ie;
					          nested_aig = 1;
						      aig2 = 1;
						 }
					}
					break;
				}

				if(n - 1)
					break;	// if ok second round for action	
				USHORT j = toff[0]; // candidate in tag form
				if(allsteps.Off(j)) { // kep only the first one
						allsteps.Set(j);
						tsets[j] = ie;
						nested_aig = 1;
						aig2 = 1;       
					}
				
				if(opp && aig2) {
					EE.E("set derive actif ");
					chx.Image(this,&EE);
					EE.Enl();}
			}
			break;
		case SET_set: // in a set all components must be on
			for(int i = 0; i < (nni - 1); i++) {  
				if(cum->On(1 + 2 * chx.tcd[i]))
					continue;
				n = 1;
				break;
			}
			if(n)
				break; // must all be there
			const EVENT &ev = tevent.t[chx.tcd[nni - 1] - event_vi];
			const EVENTLOT &evl = ev.evl;
			for(int i = 0; i < evl.itcd; i++) {
				int j = (evl.tcd[i] << 1) ^ 1; // false state on
				if(allsteps.On(j))
					continue;
				allsteps.Set(j);
				tsets[j] = ie;
				nested_aig = 1;
				aig2 = 1;       
			}
			if(opp && aig2) {
				EE.E("set actif event");
				chx.Image(this,&EE);
				ev.Image(this,&EE);
			}

			break;	
		} // end switch
	}// end proc


	if(nested_aig)
		return;
	// we look for indirect hints
	// zcf.h_one.dp.Image();dpn.Image();
	Gen_dpnShort(tag);


	BFTAG elims; 
	NestedForcingShort(elims); 

	if(elims.substract(allsteps)) { // force false so use elims.inverse()
		if(opp)
			Image(elims,"forcing chain elims" ,0);	
		allsteps |= elims; // and in the total 
		nested_aig = 1;
	}
	if(rbase < 100)
		return;

	BFTAG elims2; 
	NestedMultiShort(elims2); 
	if(opp && options.ot)
		Image(elims2,"multiforcing recap short ", 0);
	if(elims2.IsNotEmpty()) {
		allsteps |= elims2; //   in the total 
		nested_aig=1;
	}
}

/* dpnshort is pecial for level 4 (nested dynamic)
   in that level, derived weak link have to be considered 
     for the remaining valid candidates of the set

*/
 
void PUZZLE::Gen_dpnShort(USHORT tag) { // create the reduced set of tags check tagelim empty 
	dpn.Init(); 
	BFTAG * tdp = hdp_base_nested.t;
	USHORT tagc = tag;
	if(tagc & 1) tagc ^= 1;
	for(int j = 2; j < col; j++) {
		if(j == tagc)
			continue; // don't process the start point
		if(allsteps.On(j))
			continue; // that tag is defined
		if(allsteps.On(j ^ 1))
			continue; // that tag is defined as well (to check)
		dpn.t[j] = tdp[j];
		if(rbase>100) 
			dpn.t[j] |= dpn_old.t[j];
		dpn.t[j] -= allsteps; // reduce the primary weak links
	}
	if(rbase>100){  //level 4 must find  derived weak links
		// this must be a specific process working on reduced sets;
		// only one step of derivation is made 
	   // dn.ExpandAll(*this, dpn); // done in derive
        zcx.DeriveDynamicShort(allsteps,dpn,dn);
		  // do it twice
	   // dn.ExpandAll(*this, dpn); same
        zcx.DeriveDynamicShort(allsteps,dpn,dn);
		dpn_old=dpn;
 	}
	dn.ExpandAll(*this, dpn);
	if(0) {  
		Image(allsteps,"allsteps",0);
		EE.Enl("image dpn de dpnshort");
		Image(dpn);
		EE.Enl("image dp de dpshort");
		Image(dpn);
	}

}


void PUZZLE::NestedForcingShort(BFTAG & elims) {
	if(opp){ ///opp){
		EE.Enl("entry nested forcing short");
		Image(allsteps,"allsteps",0);
		Image(dn);
		Image(dpn);
	}
	for(int i = 2; i < col; i += 2) {
		if(allsteps.Off(i ^ 1) && dn.Is(i, i ^ 1))  // a forcing chain found, find the length
			elims.Set(i ^ 1); 
		if(rbase > 100){
           // look  for contradiction chains ??
		   BFTAG tw(dn.t[i]);
           tw &= (dn.t[i].Inverse()).FalseState();
		   tw -= allsteps;
		   if(tw.IsNotEmpty())
			   elims.Set(i ^ 1);

		   // and also for dual chains
	       // still  only for fresh eliminations

		   tw = dn.t[i];
           tw &= dn.t[i ^ 1].FalseState();
		   tw -= allsteps;

		   elims |= tw;
		}
	}
}

/*
Now looking for multi chains eliminations if any
the bfs must contain all candidates killed by the main assumption
and the source of all new strong links used
*/

void PUZZLE::NestedMultiShort(BFTAG & elims) {
	if(0){
		EE.Enl("entry nested multi short");
		Image(allsteps,"allsteps",0);
		Image(dn);
		Image(dpn);
	}
	for(int ie = 1; ie < zcx.izc; ie++) {
		const SET &chx = zcx.zc[ie];
		int nni = chx.ncd, aig2 = 1; 
		BFTAG zt;
		zt.SetAll_1();
		zt = zt.FalseState();
		zt -= allsteps;
		if(chx.type - SET_base)
			continue;
		// must be  n false 
		for(int i = 0; i < nni; i++) { // max one free 
			USHORT cd = chx.tcd[i], j = cd << 1; // candidate in tag form
			if(cum->On(j)) {
				aig2 = 0;
				break;
			}// set assigned
			if(cum->Off(j ^ 1))
				zt &= dn.t[j];
		}
		if(aig2)
			elims |= zt; 
	}// end ie
}
/*
    warning
	                  ================================
	that process and upstream search should be extended to dynamic forcing chain
	and dynamic nested forcing chain
	it seems this does not exist in serate so chould be an option 
	                   ==============================

Dynamic search in nested mode for a candidate
   do both contradiction, 
   direct  a -> x and a -> ~x so 'a' is false
   indirect x -> ~a   and ~x -> ~a  'a' is false
     (also x -> ~a and y -> ~a if xy is a bivalue
   and "set contradiction" 2 a for each start "true" of the set
   tagnot is
     0 if no indirect search has to be done
	 pointer to tag expanded to do a direct check

   zcf.h is refreshed for each new entry from zcf.h_one
   zcf.h.dp follows the dynamic forward process 
     each new link is loaded in the dp table
*/


int PUZZLE::GoNestedCase1(USHORT cand) {
	opp = 0; 
	//if(Op.ot && rbase>100)opp=1;
	USHORT tag = cand << 1; 
	if(rbase>90){
	   zcf.StartNestedOne();
	   zcx.StartNestedOne();
	   zcxb.StartNestedOne();
	}
	BFTAG tt = zcf.h.d.t[tag]; 

	if( opp) {
		    EE.E("go nested for cand ");
		    zpln.Image(cand);
		    EE.Enl();
	         }

	tcandgo.Init(); // intital storage for strong links and nested eliminations
	tstore.Init(); // start with an empty table of chains stored

	hdp_base_nested=zcf.hdp_base;

	//now go forward  till nothing or a contradiction

	to = zcf.hdp_dynamic.t; // we use the table without derivations
	for(int i = 0; i < 640; i++)
		tsets[i] = 0;
	npas = 0;
	steps[0].SetAll_0();
	steps[0].Set(tag); 
	allsteps = cumsteps[0] = steps[0];  
	tx[0][0] = tag;
	itx[0] = 1; // initial is tag to go forward

	//ret_code = 0;
	nested_aig = 1;
	int maxpas = pasmax;  // will be reduced to "one more" if something found

	//--------------------- loop  forward
	while(nested_aig && npas++ <= maxpas) {
		  // prepare the call to GoNestedWhileShort
		nested_aig = 0; 
		cum = &cumsteps[npas - 1];
		    // reasonnable stop for crazy cases
		    // could surely be set lower
//		if(cum->Count()>250) break; // nothing more to find /// en cours verif sur test
		step = &steps[npas];
		ta = tx[npas-1];
		ita = itx[npas - 1];


		GoNestedWhile(tag);                    // while cycle

		if(nested_aig){// something happenned
		  cumsteps[npas] = allsteps;   
		  (*step) = allsteps;
		  (*step)-=(*cum);  
		  (*step).String(tx[npas],itx[npas] );
		  }
	   
	    else 
			break; // nothing to do

		if( opp) {  
			EE.E("end step=");
			EE.E(npas);
			Image((*step),"step ", 0);
			Image(allsteps,"all", 0);
			EE.E(" tbuf.istore=");EE.E( tstore.ibuf);
			EE.E(" tbuf.ise=");EE.E( tstore.ise);
			EE.E(" tbuf.ise2=");EE.Enl( tstore.ise2);

		}

		// check for a contradiction in that lot 
		USHORT * tb=tx[npas];
		for(int i = 0; i < itx[npas]; i++) {
			USHORT tgx = tb[i];
			if(allsteps.On(tgx) && allsteps.On(tgx ^ 1)) {
				if(1 && opp) {
					EE.E("\n\nfound active a -> x and a -> ~x");
					zpln.ImageTag(tgx);
					EE.E(" step=");
					EE.Enl(npas);   
				}
				// we compute back the length and show the sequence 
				if(maxpas > npas + 2)
					maxpas = npas + 2; // limit the process to 2 more step
				if(maxpas > pasmax)
					maxpas = pasmax;
				nested_print_option = 0;
				//if(couprem==25)nested_print_option=1; 

				tstore_final.Init(); // start with an empty table of chains used
				int l1 = GoBackNested(tgx), 
					l2 = GoBackNested(tgx ^ 1);
				if(stop_rating) return 1;// push back error code 
				if((!l1) || (!l2))
					continue; // should not be
				int ratch = tchain.GetRating(l1 + l2, tag >> 1);
				if(ratch) { // chain is accepted load it (more comments in test mode)
					if(options.ot && ratch<120) { //don't develop if >119 never seen used
				        tstore_final.Init(); // start with an empty table of chains used
						nested_print_option=1;
						EE.E("\n\nchain plus killing ");
						zpln.Image(tag >> 1);
						EE.E(" computed length = ");
						EE.Enl(l1+l2);	     
						EE.E("chain 1 ");
						GoBackNested(tgx);
						EE.E("chain 2 ");
						GoBackNested(tgx ^ 1);
					}
					tchain.LoadChain(ratch, "chain plus", cand);
				}// end load chain
              // stop as soon as elimination has been done
             if(tchain.cycle_elims.On(cand)) break;
			} // end one source for elimination
		}// end for i
		if(0 && !nested_aig) {
			EE.E("fin aig=0 pour step=");
			EE.Enl(npas);
		}
              // stop as soon as elimination has been done
     if(tchain.cycle_elims.On(cand)) break;
	}// end while
	return 0; //ret_code;
}

/* 
Dynamic search in nested mode for a candidate
   cleared through a set  of  candidates
 
 ttags:  the sets or the 2 states in tag form
 ntags   the number of tags to process
 target  the tag to eliminate (candidate in false state)
 base     the base depending on the level 

 expand each tag of ttags till target is found
*/
/* find the target in dynamic mode for a set of tags

  pending optimisation on bi values with ntags==2
  if base = 75 (Nishio), nothing to do
  if base = 85, it is always a bi value
  if Base > 85 reduce by one if
    first step in chain only one candidate and a "true" form

*/

/// en cours travail

void PUZZLE::Rating_Nested( USHORT * ttags, USHORT ntags, USHORT target) {
	opp=0;
	//if(ntags==3 && couprem<9 && target) opp=1; else opp=0;   
	if(opp){
		EE.E("\n\nrating nested entry for "); zpln.ImageTag(target);  
		EE.E( "  through  " );
		for(int it=0;it<ntags;it++){			 
		      zpln.ImageTag(ttags[it]);
		      EE.Esp();
		}
	}
	USHORT ctarg = target >> 1;

	// forget if target already eliminated
	if(tchain.cycle_elims.On(ctarg)) return;

	// filter if target if part of the set
	for(int i = 0; i < ntags; i++)
		if(ttags[i] >> 1 == ctarg)
			return;	

	USHORT length = 0;
	if((ntags==2) && (rbase==85) )length--;
	// to revise later cell or region bi values instead of that if valid


	nested_print_option= 0;
	if(opp)nested_print_option= 1;
	tstore_final.Init(); // start with an empty table of used chains
	for (int i = 0; i < ntags; i++) {
		USHORT lx = GoNestedCase2_3( ttags[i], target);
		if(opp){///
			EE.E("\n\nlength received= "); EE.Enl(lx); 
		}

		if(stop_rating) 
			return ;// push back error code 
		if(!lx)
			return; //should never be
		length += lx;
		  // in next call use ls to avoid length <2
	    int ratch = tchain.GetRating(lx, target >> 1);
		if(!ratch) break; // stop as soon as possible
	}

	int ratch = tchain.GetRating(length, target >> 1);

	if(ratch) { // chain is accepted load it (more comments in test mode)
		if(options.ot) { 
			EE.E("\n\nrating nested killing "); zpln.ImageTag(target);  
			EE.E( "  through  " );
			for(int it=0;it<ntags;it++){			 
		       zpln.ImageTag(ttags[it]);
		       EE.Esp();
			}
			EE.Enl();	  
			// we restart the process in print mode

	        nested_print_option=1;
			tstore_final.Init(); // start with an empty table of used chains
	        for (int i = 0; i < ntags; i++) {		
		         GoNestedCase2_3( ttags[i], target);		 
	        }
		}
		tchain.LoadChain(ratch, "chain plus through set", target >> 1);
	}// end if
}

int PUZZLE::GoNestedCase2_3( USHORT tag, USHORT target) {
	if(rbase>90){
	  zcf.StartNestedOne();
	  zcx.StartNestedOne();
	  zcxb.StartNestedOne();
	}

	if(opp) {
		EE.E("go nested case 2_3for tag ");
		zpln.ImageTag(tag);
		EE.E(" target= ");
		zpln.ImageTag(target);
		EE.E(" print_option= ");
		EE.Enl(nested_print_option);
	}

	tcandgo.Init(); // intital storage for strong links and nested eliminations
	tstore.Init(); // start with an empty table of chains stored
	hdp_base_nested=zcf.hdp_base;

	//now go forward  till nothing or a contradiction

	to = zcf.hdp_dynamic.t; // we use the table without derivations
	for(int i = 0; i < 640; i++)
		tsets[i] = 0;
	npas = 0;
	steps[0].SetAll_0();
	steps[0].Set(tag); 
	allsteps = cumsteps[0] = steps[0];  
	tx[0][0] = tag;
	itx[0] = 1; // initial is tag to go forward

	nested_aig = 1;
	int maxpas = pasmax;  

	//--------------------- loop  forward
	while(nested_aig && npas++ <= maxpas) {
		nested_aig=0; 
		cum = &cumsteps[npas-1];
		step = &steps[npas];
		step->SetAll_0();
		ta = tx[npas-1];
		ita = itx[npas-1];

		GoNestedWhile(tag);                    // while cycle
		if(nested_aig){// something happenned

 		  cumsteps[npas] = allsteps;   
		  (*step) = allsteps;
		  (*step)-=(*cum);  
		  (*step).String(tx[npas],itx[npas] );
		  }
	   
	    else 
			break; // nothing to do


		if(opp){
			EE.E("fin step=");
			EE.E(npas);
			Image((*step),"step ", 0);
			Image(allsteps," allsteps",0);
		}

		// check for a contradiction in that lot (stop at first)
		if(allsteps.On(target)) {
			// we compute back the length and show the sequence 
			int l1 = GoBackNested(target);
			return l1;
		}

	}// end while
	return 0;
}

/* while cycle  for GoNested 
   first apply new actions from direct implications and sets
   then check if new strong links appeared
   if new strong links, search for new indirect forcing chain
   if level, check anyway new multi chain eliminations 
*/
void PUZZLE::GoNestedWhile(USHORT tag) {
	USHORT aignl = 1;
	BFTAG  * hdpb = hdp_base_nested.t; // to receive new strong links
	// look first for direct 
	for(int it = 0; it < ita; it++) {
		BFTAG x = to[ta[it]];
		if(x.substract(allsteps)) {
			if(opp)
				Image(x,"applied std", ta[it]);
			allsteps |= x; // and in the total 
			nested_aig=1;
		} 
	}

	// check now sets

	for(int ie = 1; ie < zcx.izc; ie++) {
		const SET &chx = zcx.zc[ie];
		int n = 0, nni = chx.ncd, aig2 = 0;
		//int toff[10];
		int toff[3] = {0,0,0};
		switch (chx.type) {
		case SET_base:  // must be n-1 false or n false (then all is ok)
			{
				// check if active 0 to 2 cand unknown
				BFTAG bfw;
				for(int i = 0; i < nni; i++) { // max one free 
					USHORT cd = chx.tcd[i], j = cd << 1; // candidate in tag form
					if(cum->On(j)) {
						n = 3;
						break;
					}// set assigned
					if(cum->Off(j ^ 1)) {
						toff[n++] = j;
						if(n > 2)
							break;
					} 
					else
						bfw.Set(j ^ 1);
				}

				USHORT cd1 = toff[0], cd2 = toff[1];
				if(n == 2) {    // this is a new strong link
                  if(rbase>90){  // only for nested
					if(hdpb[cd1].Off(cd2 ^ 1) || 
					   hdpb[cd2].Off(cd1 ^ 1) || 
					   hdpb[cd1 ^ 1].Off(cd2) || 
					   hdpb[cd2 ^ 1].Off(cd1)) {

						hdpb[cd1].Set(cd2 ^ 1);
						hdpb[cd2].Set(cd1 ^ 1);
						hdpb[cd1 ^ 1].Set(cd2);
						hdpb[cd2 ^ 1].Set(cd1);
						aignl = 0;
						tcandgo.AddStrong(cd1 >> 1, cd2 >> 1, bfw, nni - 2);
                        nested_aig=1;// force new loop if new strong link
					}
				  }
					break;
				}
				if(!n){  // nothing valid switch to a contradiction on any candidate
                    for(int i = 0; i < nni; i++) { 
                         USHORT cd = chx.tcd[i], j = cd << 1;
						 if(allsteps.Off(j)) {
						      allsteps.Set(j);
						      tsets[j] = ie;
					          nested_aig = 1;
						      aig2 = 1;
						 }
					}
					break;
				}

				if(n - 1)
					break;	// if ok second round for action	
				USHORT j = toff[0]; // candidate in tag form
				if(allsteps.Off(j)) {
						allsteps.Set(j);
						tsets[j] = ie;
						nested_aig = 1;
						aig2 = 1;
					}
				
				if(0 && aig2) {
					EE.E("set derive actif ");
					chx.Image(this,&EE);
					EE.E("  valid ");
					zpln.ImageTag(j);
					EE.Enl();
				}
			}
			break;
		case SET_set : // in a set all components must be on
			if(rbase<80) break; // not for Nishio

			for(int i = 0; i < (nni - 1); i++) {
				if(cum->Off((chx.tcd[i] << 1) ^ 1)) {
					n++;
					if(n)
						break;
				}
			}
			if(n)
				break; // must all be there
			const EVENT ev = tevent.t[chx.tcd[nni - 1] - event_vi];
			const EVENTLOT &evl = ev.evl;
			for(int i = 0; i < evl.itcd; i++) {
				int j = (evl.tcd[i] << 1) ^ 1; // false state on
				if(allsteps.On(j))
					continue;
				allsteps.Set(j);
				tsets[j] = ie;
				nested_aig = 1;
				aig2 = 1;       
			}
			if(0 && aig2) {
				EE.E("set actif event");
				chx.Image(this,&EE);
				ev.Image(this,&EE);
			}
			break;	
		} // end switch
	}// end proc

	// stop if not nested mode or something found

	if((rbase<95) || nested_aig)
		return;    

	// we look for indirect hints
	Gen_dpn(tag);    // create a fresh reduced set of tags 
	// zcf.h_one.dp.Image();dpn.Image();
	BFTAG elims; 
	if(rbase<105)
		NestedForcing(elims); 
	else
	    NestedForcingLevel4(elims); 
	if(opp && options.ot) {
		Image(allsteps,"allsteps", 0);
		Image(elims,"netforcing recap", 0);
	}

	//don't do eliminations now, it creates troubble later

	if(!(rbase < 100 || elims.Count() > 20) ){// limit 20 is realism  
		BFTAG elims2; 
		if(rbase < 105)
			NestedMulti(elims2); 
		else
			NestedMultiLevel4(elims2); 
		if(opp && options.ot)
			Image(elims2,"multiforcing recap", 0);
		if(elims2.IsNotEmpty()) {
			allsteps |= elims2; //  in the total 
			nested_aig=1;
		}   
	}

	if(elims.IsNotEmpty()) {
		allsteps |= elims; // and in the total 
         //		to study that puzzle generating a huge elims
		// 1....67...571.......9....1..4....3.......8..29..7...6......24..5..6...9.....3...8;11.40;10.70;10.00
		nested_aig = 1;
	}

}

/* looking for fresh forcing chain
   we have to find
   the equivalent length
   all data necessary to track back the path, which means
   all candidates that must be eliminated to generate new strong links in use in the chain
 
  to explain the path
    go bakcward
    take all even steps (start=1) which are strong links
    check if this is a new link if yes take the equivalent BFTAG

*/
void PUZZLE::NestedForcing(BFTAG & elims) {

	for(int i = 2; i < col; i += 2) {
		if( dn.Is(i, i ^ 1)) {  // a forcing chain found, find the length
			BFTAG wch = dpn.t[i], bfs; 
			int era=0;
			USHORT tt[100], itt ; 
			int npasch = wch.SearchChain(dpn.t, i, i ^ 1);
			if((!npasch)|| npasch > 40){
				era=1;
				continue; // never seen so far
			}
			
			else{// 
               itt = npasch + 2;
			   if(wch.TrackBack(dpn.t, i, i ^ 1, tt, itt, i ^ 1))
			       era=1;   // intercept error for debugging	
			}
			
			if(era){// never seen so far
					continue; // just skip it
			}
			  //must add the source of the new strong links
			for(int j = 1; j < itt - 1; j++) { // all strong links 
				if((!(tt[j] & 1)) || (tt[j + 1] & 1)) continue;
				const CANDGOSTRONG * w = tcandgo.Get(tt[j], tt[j + 1]);
				if(w) bfs |= w->source;// it is a new strong link
			}
			USHORT ii = tstore.AddOne(tt, itt);
			tsets[i ^ 1] =- tcandgo.itt;
            elims.Set(i ^ 1); 

			tcandgo.tt[tcandgo.itt++].Add(ii, bfs, npasch + 1); 
		}// this is the final length

	}
}

/*
Now looking for multi chains eliminations if any
the bfs must contain all candidates killed by the main assumption
and the source of all new strong links used

*/
void PUZZLE::NestedMulti(BFTAG & elims) {
	for(int ie = 1; ie < zcx.izc; ie++) {
		const SET &chx = zcx.zc[ie];
		int nni = chx.ncd, aig2 = 0; 
		BFTAG zt(BFTAG::InitFalseState);

		BFTAG ttt = allsteps;
		ttt |= elims;
		zt -= ttt;
		if(chx.type - SET_base)
			continue;
		// must be  n false 
		for(int i = 0; i < nni; i++) { // max one free 
			USHORT cd = chx.tcd[i], j = cd << 1; // candidate in tag form
			zt.Clear(j^1);    // don't keep eliminations if belonging to the set
			if(cum->On(j)) {
				aig2 = 1;
				break;
			}// set assigned
			if(cum->Off(j ^ 1))
				zt &= dn.t[j];
		}
		if(aig2 )
			continue;	// if ok second round for action	

		USHORT p[640],np;
		zt.String(p,np);
		for(int ip = 0; ip < np; ip++) {
			int i=p[ip];
			BFTAG bfs;
			USHORT istored = 0, istoref = 0, tot_count = 0;
			int era=0; // one error somewhere debugging data
			for(int i2=0;i2<nni;i2++) {
				USHORT cd = chx.tcd[i2], j = cd << 1; // candidate in tag form
				if(cum->On(j ^ 1)) {
					bfs.Set(j ^ 1);
					continue;
				}// already false
				if(allsteps.On(j^1)){ // this creates a problem, stop the process
					era=2;
					break;
				}
				// here can be direct and this is not done in search chain
				// dummy cycle if direct to have common process

				BFTAG wch = dpn.t[j]; 
				USHORT tt[50], itt = 2; tt[0] = j; tt[1] = i;
				if(wch.Off(i)) {

					int erb=0,  // to catch an error in that sequence
					npasch = wch.SearchChain(dpn.t, j, i);	
					if((!npasch) || (npasch > 40))
						erb=1;  // should never happen  debug later

					else{

						itt = npasch + 2; 
					  if(wch.TrackBack(dpn.t, j, i, tt, itt, i)) 
						  erb=2;// intercept error for debugging
					}

					if(erb) { // add some debugging code
						EE.E("nested multi erb="); EE.E(erb);
						EE.E(" npasch="); EE.E(npasch);EE.Esp();
						chx.Image(this,&EE);
						EE.E( " search "); zpln.ImageTag(j); EE.Esp();
						EE.E( " => "); zpln.ImageTag(i); EE.Enl();
						Image((*cum),"cum ",0);
						Image(dpn.t[j],"dpn (j)",0);
						Image(dn.t[j],"dn (j)",0);
						EE.Enl();
						era=1;
					}

					else {
				
				    // must add the source for the new strong links
				    for(int j = 1; j < itt - 1; j++) { // all strong links 
					   if((!(tt[j]&1)) || (tt[j+1]&1))
						   continue;
					   const CANDGOSTRONG * w = tcandgo.Get(tt[j], tt[j+1]);
					   if(w) bfs |= w->source;// it is a new strong link		
				    }
				    istoref = tstore.AddChain(tt, itt);
				    if(!istored)
					   istored = istoref;
				   tot_count += itt;

			       }// end else not erb
				}  //end if
			} // end i2

            if(era){ 
				if(era>1)
					continue; // process stopped after candidate killed in forcing chains
				
				// stop and log message to see
 			  stop_rating=1;
			  elims.SetAll_0();
			  return;
			}

			 // here a case to be investigated deeper
			 //.2..5......71....668.......2..7....8..8...3....1..4.9..1.2....7....9.4.......3.5. ED=0.0/0.0/0.5
			 // no error in detailed chains
			 // but nothing stored, so all was defined ???
			 // good example to check the process

			if(istored) { //should always be
				USHORT ii = tstore.AddMul(istored, istoref);
				tsets[i] =- tcandgo.itt;
				elims.Set(i); 
				tcandgo.tt[tcandgo.itt++].Add(ii, bfs, tot_count); 

                if(opp)  // print it it test mode
	             {EE.E("new eliminated");   zpln.ImageTag(i); 
		          EE.E("  ichain="); EE.E(tstore.ise); 
		          EE.E("  stored as "); EE.Enl(tsets[i]); 
                 }
			}
		} // end tag i
	}// end ie
}

/*  create a reduced tdb without
    the candidate sutdied
	all candidates forced or cleared if the candidate studied is true
	including all new strong links generated
	and eliminations depending on the level

	at level 4 the process is dynamic
	so the chain must include derived links
*/
///
void PUZZLE::Gen_dpn(USHORT tag)
{          // create the reduced set of tags check tagelim empty 
 dpn.Init(); 
 BFTAG    * tdp=hdp_base_nested.t;


 for (int j=2;j< col;j++) 
	{if(j==tag) continue; // don't process the start point
	 if(allsteps.On(j)) continue; // that tag is defined
	 if(allsteps.On(j^1))
		 continue; // that tag is defined as well (to check)
      dpn.t[j] = tdp[j];
	  dpn.t[j] -= allsteps; // reduce the primary weak links
	 }
  if(0)   {Image(allsteps,"allsteps at gen time",0);
		   Image(dpn);
          }
 if(rbase>100){  //level 4 must find  derived weak links
	// this must be a specific process working on reduced sets;
	// only one step of derivation is made to start
	// must be the same as in dpn short
	   chain4_dpn=dpn; //store dpn for nested dynamic expansion

       zcx.DeriveDynamicShort(allsteps,dpn,dn);
       zcx.DeriveDynamicShort(allsteps,dpn,dn);// do it twice
       zcx.DeriveDynamicShort(allsteps,dpn,dn);// may be 3 times

 }
  dn.ExpandAll(*this, dpn);
 }

//--------------------------------------------------
int PUZZLE::GoBackNested(USHORT tag) {
	if(0 && nested_print_option ) {
		EE.E("goback");zpln.ImageTag(tag);
		EE.E(" npas=");EE.Enl(npas);
	}

	USHORT itret1=0,nestedlength=0;
	itret=0;
	BFTAG bf; 
	tret[itret++] = tag;
	bf.Set(tag);
	while(itret1 < itret && itret < 300) { // solve each entry back
		USHORT x = tret[itret1], aig = 1; // first locate where x has been loaded
		int index = tsets[x];
		if(0 && nested_print_option) {  
			EE.E("go back look for ");
			zpln.ImageTag(x);
			EE.E(" index= ");EE.E( index);
			EE.E(" itret1=");EE.E(itret1);EE.E(" itret=");EE.Enl(itret);
		}
		for(int i = 0; i <= npas; i++) {
			if(steps[i].On(x)) {
				aig=0; 
				if(i) {  // not initial assumption
	              if(!index) { // this is a direct step
	                 int z=0,ia=i-1;

		             // take a parent already there in priority
		             // note  this should be extended to SET_sets

	                 for(int i2=0;i2<=i;i2++){
		                for(int j=0;j<itx[i2];j++){
	                       USHORT y=tx[i2][j]; 
			               if(to[y].On(x) && bf.On(y)) {
					         z=y;break;
			               }	
			            }
			            if(z)break;
		             }
	                 if(!z )  { // none already there, take the earliest possible

	                  for(int i2=0;i2<=i;i2++)  {

				         // prospect that step

			            for(int j=0;j<itx[i2];j++){ 
	                        USHORT y=tx[i2][j]; 
			                if(to[y].On(x))  {
					           z=y; 
                               tret[itret++]=z;
			                   bf.Set(z);			          
					           break;			   
					         } // end if
	                    }// end step prospect

				        if(z)   break;  // stop at earliest stepfound

			            }// end loop on steps

			          } // end if(!z)

			          if(!z) {  // not found should never be
						  aig=1;
						  if(options.ot){// debugging sequence
                             EE.E("debug for goback direct not found target ");
							 zpln.ImageTag(tag);EE.E(" npas=");EE.Enl(npas);
							 EE.E(" x= "); zpln.ImageTag(x);
							 EE.E(" istep");EE.Enl(i);

							 for (int iw=0;iw<=i;iw++) {
								  EE.E(iw); Image(steps[iw]," step",0);
							 }
                             for (int iw=0;iw<=i;iw++) {
								    EE.E(iw); EE.E(" step tx "); 
								    for(int j=0;j<itx[iw];j++){ 
								    zpln.ImageTag(tx[iw][j]);
								    EE.Esp();
									}
									EE.Enl();
							 }							 
							 Image(zcf.h.dp);
							 EE.Enl("\n tsets table");
							 for(int iw=2;iw<640;iw++) {
								 if(cumsteps[i].On(iw)) {
									 zpln.ImageTag(iw);
									 EE.E(" tset ");
									 EE.Esp();
									 EE.Enl(tsets[iw]);
								 }
							 }
						  }
					  }
		           } // end index=0

	   			   else if(index > 0) {
						// it comes from a set, we know which one
						//   but  may be a shorter path using anoter set
						// we take the shortest size giving that candidate 
						//   using found candidates (false)  (to code)
						SET chx = zcx.zc[tsets[x]];
						if(0 && nested_print_option) {
							EE.E("set");
							chx.Image(this,&EE);
							EE.Enl();
						}
						int n = chx.ncd; 
						if(chx.type == SET_set)
							n--; // for a set, forget the event
						else if(n > 3) { // if n>3 try to find a shorter set
							for(int j = tsets[x] + 1; j < zcx.izc; j++) {
								SET chxj = zcx.zc[j];
								if(chxj.type == SET_set)
									break; // only sets
								int nj = chxj.ncd; 
								if(nj >= n)
									continue; // keep the first lowest only
								int aig = 0;
								for(int k = 0; k < nj; k++) {
									USHORT y = chxj.tcd[k] << 1;
									if(cumsteps[i].On(y ^ 1))
										continue;
									if(y == x)
										aig = 1; // must be 'x' onece
									else
										aig = 0;
									    break;
								}
								if(aig) {
									n = nj;    // replace the set by the new one
									chx = chxj;
									if(n == 3)
										break;  // stop at first 3 cand reached
								}
							} // end j
						}  // end if n>3

						for(int j = 0; j < n; j++) {
							USHORT y = chx.tcd[j] << 1;
							if(y == x)
								continue;
							y ^= 1;
							if(bf.Off(y)) {
								tret[itret++] = y;
								bf.Set(y);
							}
						}
					}

				       // when we have a nested elimination, we have to check
				       //whether it has already been used;
				       // if yes, the extra count is set ot 0
					else {  // index <0 this is a nested elimination
						CANDGOFORWARD w = tcandgo.tt[-index];
						int ir=tstore_final.Use(tstore,w.index);
						if(ir){
							nestedlength += w.count;
							if(0){///nested_print_option && options.ot && couprem==7){
								EE.E("counted as new ir=");EE.Enl( ir);	
								tstore.Print(this,&EE,w.index);
								if(ir>100){
									tstore_final.Print(this,&EE,ir-101);
								}
							}
						}
						else {
							if(0){///nested_print_option && options.ot&& couprem==7){
								EE.Enl("counted for 0");	
								tstore.Print(this,&EE,w.index);
							}
						}
						BFTAG bfn( w.source);
						bfn -= bf; // find new tags needed
						USHORT newCount;
						bfn.String(&tret[itret], newCount);// put them in the list "to explain"
						itret += newCount; // and adjust the count
						bf |= bfn;  // update the list of tags icluded
						if(nested_print_option && options.ot&& couprem==7){ 
							EE.Enl("nested elimination");	
                            tstore.Print(this,&EE,w.index);
							Image(bfn,"new tags needed",0);
							EE.Enl();
						}
					}
				}
			i = 100;// force end of process after it has been found

			}
		}  // end i
       if(aig || itret>300) {
	       stop_rating=1;
	        if(options.ot) {
				EE.E("go back nested invalid situation itret1=");
			    EE.E(itret1);
			    EE.E(" itret=");
			    EE.E(itret);
				EE.E(" x=");zpln.ImageTag(x);
				EE.E(" index=");EE.E(index);
				EE.E(" goback");zpln.ImageTag(tag);
		        EE.E(" npas=");EE.Enl(npas);
				for(int i=0;i<=npas;i++){
					EE.E(i);
					Image(steps[i],"step",0);
                    EE.Enl();				
				}

		    }

	       opp=0;
	       return 0; // not found, should never be
       }	
   	   itret1++;
		if(0 && options.ot) {
			EE.E("go back end step   itret1=");
			EE.E(itret1);
			EE.E(" itret=");
			EE.Enl(itret);
		}
	}
	if(nested_print_option && options.ot) { // printing in increasing order of generation
		EE.Enl(" eliminations justification ");
		for(int i = 0; i <= npas; i++) {
			for(int j = 0; j < itret; j++) { 
				if(steps[i].On(tret[j])) {
					USHORT wt = tret[j]; 
					zpln.ImageTag(wt); // print the tag and look for explanation
					int index = tsets[wt];
					if(!index)
						EE.Enl();  // direct no comment
					else if(index > 0) { // it comes from a set, we know which one
						SET chx = zcx.zc[index];
						EE.E(" through set ");
						chx.Image(this,&EE);
						EE.Enl(); 	
						if(chx.type == SET_set){// then print more
							int ev=chx.tcd[chx.ncd-1]- event_vi;
							tevent.t[ev].ImageShort(this,&EE);
						}

					}
					else {  // index <0 this is a nested elimination
						CANDGOFORWARD w = tcandgo.tt[-index];
						EE.E(" through nested chain(s)  count=");
						EE.Enl(w.count);
						tstore.Print(this,&EE,w.index);
					}
					EE.Enl();
				}
			}
		}
		EE.E("return itret=");
		EE.E(itret);
		EE.E(" nestedplus=");
		EE.Enl(nestedlength);
	}
	 
	 // this is the right place for a bi value adjustment
	 // bi value if start false
	 //   and "step one and result only one true in common"
	 // this is processsed by serate as a cell or region bivalue
	 int biv=0;
	 if(tx[0][0] ^1) {// this is a false start
	      BFTAG bw(bf);
	      bw &= steps[1];
		  if(bw.Count()==1 ){ // must be one used in the first step
			  USHORT tw[2],itw;
			  bw.String(tw,itw);
			  if(!(tw[0] ^1))  // must be a true
			     biv=-1;
		  }
	 }

	return itret + nestedlength ; //+ biv; waiting examples for validation
}


///

void PATH::PrintPathTags(CANDIDATES * zpln) {
	zpln->PrintImply(pth, ipth);
}

/*  process to find a nested chain in dynamic mode (level 4)
    looking for forcing or contradiction
    only basic strong and weak links
	but dynamic search (using basic sets)

*/
int PUZZLE::CaseNestedLevel4Case1( USHORT tag )  {
	if(0) {
		EE.E("\n\nnested level 4 case 1 for tag ");
		zpln.ImageTag(tag);
		EE.Enl();
	}

	chain4_to = chain4_dpn.t; // we use the table  for nested
	for(int i = 0; i < 640; i++)
		chain4_tsets[i] = 0;
	chain4_npas = 0;
	chain4_steps[0]=(*cum);  // start with the known status
	chain4_steps[0].Set(tag);  // plus the tag
	chain4_allsteps = chain4_cumsteps[0] = chain4_steps[0];  
	chain4_buf[0]=tag;        // and expand the tag in cycle one
	chain4_tx[0] = chain4_buf;
	chain4_itx[0] = 1; // initial is tag to go forward

	nested_aig2 = 1;
	
	//--------------------- loop  forward
	while(nested_aig2 && chain4_npas++ <= pasmax) {
		nested_aig2=0; 
		chain4_cum = &chain4_cumsteps[chain4_npas-1];
		chain4_step = &chain4_steps[chain4_npas];
		chain4_ta = chain4_tx[chain4_npas-1];
		chain4_ita = chain4_itx[chain4_npas-1];
		chain4_tx[chain4_npas]=&chain4_ta[chain4_ita];

		NestedChainWhile(tag);                    // while cycle

		if(!nested_aig2)
			return 0;			// nothing happenned finished should not be
   
		chain4_cumsteps[chain4_npas] = chain4_allsteps;   
		(*chain4_step) = chain4_allsteps;
		(*chain4_step)-=(*chain4_cum);  
		(*chain4_step).String(chain4_tx[chain4_npas],chain4_itx[chain4_npas] );
        if(0){  
		  EE.E(chain4_npas);EE.E("pas ncase lvl4 ");Image((*chain4_step),"step",0);
		  Image(chain4_allsteps,"chain4_allsteps",0);
		  Image(allsteps,"allsteps",0);
		  EE.Enl();
		}

	    if((*chain4_step).IsEmpty() ||chain4_npas>30 ){
        EE.Enl("check if null or 30 pas");
		return 0;
		}

		// stop at the first contradiction reached
		BFTAG tw(chain4_allsteps);
		tw &=(tw.Inverse()).TrueState();
		USHORT ttw[640],ittw;
		tw.String(ttw,ittw);
		if(!ittw)
			continue; 
		 
		// now we have a contradicton or a forcing chain
		// we don't look for more

		if(0){ 
               EE.Enl("contradiction reached step"); EE.Enl( chain4_npas);
			   Image(tw,"for targets",0);			  
			}
		   
		// we have to take the shortest forcing or contradiction chain
		USHORT totlength=10000;
	    for(int iw=0;iw<ittw;iw++){
			USHORT y=ttw[iw]^1; // start with false for forcing chain
			USHORT itt=NestedChainGoBack(y),
				   *tt=chain4_result;
			if(itt<2) continue; // should always be ok if 1, target already false, don't do
			chain4_bf=back4_bfsource;

			y^=1;// now y is true and can be the tag
			if(y==tag){   //this is a forcing chain its ok
			   USHORT length=chain4_iresult-1;
			   if(length<totlength){ // take it if lower
				   totlength=length;
				   USHORT im = tstore.AddOne(chain4_result,chain4_iresult);
				   tsets[tag ^ 1] =- tcandgo.itt;
				   tcandgo.tt[tcandgo.itt++].Add(im, chain4_bf,length); 
			   }
				 //   elims.Set(i ^ 1); 
				continue;
			}
			USHORT length=chain4_iresult;
			USHORT  ii=tstore.AddChain(chain4_result,chain4_iresult);

			itt=NestedChainGoBack(y); // second chain
			if(!itt) continue; // should always be ok
			length+=chain4_iresult; // add second length
			chain4_bf|=back4_bfsource;  // and new source

			if(length<totlength){ // take it if lower
				totlength=length;
				USHORT  jj=tstore.AddChain(chain4_result,chain4_iresult);
			    USHORT im = tstore.AddMul(ii, jj);
			    tsets[tag^1] =- tcandgo.itt;
				tcandgo.tt[tcandgo.itt++].Add(im, chain4_bf,length); 
			}

		}
		if(0 && totlength<10000 ){
			EE.E("\n\nnested level 4 case 1 for tag ");
			zpln.ImageTag(tag);
			EE.E("cleared length ");
			EE.Enl(totlength);

		}
		return (totlength<10000);
	}// end while
	return 0;
}


/*  process to find a nested chain in dynamic mode (level 4)
    only basic strong and weak links
	but dynamic search (using basic sets)

*/
int PUZZLE::CaseNestedLevel4( USHORT tag,USHORT target)  {

	if(0) {
		EE.E("\n\nnested level 4 for tag ");
		zpln.ImageTag(tag);
		EE.E("  target ");
		zpln.ImageTag(target);
		//Image(allsteps,"allsteps depart",0);
		EE.Enl();
	}

	chain4_to = chain4_dpn.t; // we use the table  for nested
	for(int i = 0; i < 640; i++)
		chain4_tsets[i] = 0;
	chain4_npas = 0;
	chain4_steps[0]=(*cum);  // start with the known status
	chain4_steps[0].Set(tag);  // plus the tag
	chain4_allsteps = chain4_cumsteps[0] = chain4_steps[0];  
	chain4_buf[0]=tag;        // and expand the tag in cycle one
	chain4_tx[0] = chain4_buf;
	chain4_itx[0] = 1; // initial is tag to go forward

	nested_aig2 = 1;
	
	//--------------------- loop  forward
	while(nested_aig2 && chain4_npas++ <= pasmax) {
		nested_aig2=0; 
		chain4_cum = &chain4_cumsteps[chain4_npas-1];
		chain4_step = &chain4_steps[chain4_npas];
		chain4_ta = chain4_tx[chain4_npas-1];
		chain4_ita = chain4_itx[chain4_npas-1];
		chain4_tx[chain4_npas]=&chain4_ta[chain4_ita];

		NestedChainWhile(tag);                    // while cycle

		if(!nested_aig2)
			return 0;			// nothing happenned finished should not be
   
		chain4_cumsteps[chain4_npas] = chain4_allsteps;   
		(*chain4_step) = chain4_allsteps;
		(*chain4_step)-=(*chain4_cum);  
		(*chain4_step).String(chain4_tx[chain4_npas],chain4_itx[chain4_npas] );
        if(opdiag){   
		  EE.E(chain4_npas);EE.E("pas ncase lvl4 ");Image((*chain4_step),"step",0);
		  Image(chain4_allsteps,"chain4_allsteps",0);
		  Image(allsteps,"allsteps",0);
		  EE.Enl();
		}

	    if((*chain4_step).IsEmpty() ||chain4_npas>30 ){
        EE.Enl("check if null or 30 pas");
		return 0;
		}

		 
		// check if target has been reached
		if(chain4_allsteps.On(target)){ 

			if(0){ 
               EE.Enl("target reached step"); EE.Enl( chain4_npas);
			   EE.E(" target ="); zpln.ImageTag(target);EE.Esp();
			   if(chain4_npas==1){
				   Image(chain4_steps[1],"step1",0);
				   Image(chain4_steps[0],"step0",0);
				   Image(chain4_allsteps,"chain4_allsteps",0);
 				   Image(allsteps,"allsteps",0);
				   stop_rating=1;
 			   }
			}
			// we compute back the length and  the sequence 
			// we store the chain  if ok
			USHORT itt=NestedChainGoBack(target);
			if(!itt) return 0; // should always be ok

			chain4_bf|=back4_bfsource;		
			return itt;
		}
	}// end while
return 0;}

/*
  NestedChainWhile sub function of NestedLevel4(=
  */

void PUZZLE::NestedChainWhile(USHORT tag) {
	// look first for direct 
	for(int it = 0; it < chain4_ita; it++) {
		BFTAG x = chain4_to[chain4_ta[it]];
		if(x.substract(chain4_allsteps)) {
			chain4_allsteps |= x; //  in the total 
			nested_aig2=1;
		} 
        
	}

	// check now sets and only Set_base

	for(int ie = 1; ie < zcx.izc; ie++) {
		const SET &chx = zcx.zc[ie];
		if(chx.type-SET_base)return;  // finished
		int n = 0, nni = chx.ncd, joff=0; 

		for(int i = 0; i < nni; i++) { // max one free 
					USHORT cd = chx.tcd[i], j = cd << 1; // candidate in tag form
					if(chain4_allsteps.On(j)) {
						n=2;
						break;;  // set assigned force next
					}
					if(chain4_allsteps.Off(j ^ 1)) {
						if(n++) // only one, exit with 2
							break; 
						joff = j;
					} 
				}
		if(!n){ // if nothing set all
			for(int i = 0; i < nni; i++) { 
				USHORT cd = chx.tcd[i], j = cd << 1; 
				if(chain4_allsteps.Off(j )) {
		            chain4_allsteps.Set(j);
		            chain4_tsets[j] = ie;
		            nested_aig2 = 1;
				}
			}
			continue;
		}
	
		if((n - 1) || (!joff))
			continue;	// if ok second round for action	
		chain4_allsteps.Set(joff);
		chain4_tsets[joff] = ie;
		nested_aig2 = 1;
	}// end ie

}

//--------------------------------------------------
int PUZZLE::NestedChainGoBack(USHORT tag) {
	int waig=0;
	if(opdiag) {
		waig=1;
		EE.E("goback");zpln.ImageTag(tag);
		EE.E(" npas=");EE.E(chain4_npas);
		EE.E(" source");  zpln.ImageTag(chain4_tx[0][0]);  EE.Enl();
		Image(chain4_cumsteps[0],"depart",0);
        EE.Enl();
	}

	USHORT tret[300],itret=0,itret1=0;
	back4_bfcount.SetAll_0(); 
	back4_bfsource=back4_bfcount;
	back4_bfcount.Set(tag);
	tret[itret++] = tag;
	chain4_steps[0]=chain4_cumsteps[0]; // insert the full start
	while(itret1 < itret && itret < 300) { // solve each entry back
		USHORT x = tret[itret1], aig = 1; // first locate where x has been loaded
		int index = chain4_tsets[x];
		if(waig){
		    EE.E("ncgb step index=");EE.E(index);
		    EE.E(" pour tag ");		zpln.ImageTag(x);
		    EE.E(" itret=");EE.E(itret);
		    EE.E(" itret1=");EE.E(itret1);
		    EE.Enl();
		}
		for(int i = 0; i <= chain4_npas; i++) {
			if(chain4_steps[i].On(x)) {
				aig=0; 
				if(i) {  // not initial assumption
	              if(!index){ // this is a direct step
	                 int z=0,ia=i-1;
		             for(int j=0;j<chain4_itx[ia];j++){ 
	                        USHORT y=chain4_tx[ia][j]; 
			                if(chain4_to[y].On(x))  {
					           z=y; 
							   if(back4_bfcount.Off(z)){// not yet there
                                  tret[itret++]=z;
			                      back4_bfcount.Set(z);
								  // add source for new strong links
								  if((y&1) && zcf.hdp_base.t[y].Off(x)){
									  // this must be a new strong link
									const CANDGOSTRONG * w = tcandgo.Get(y, x);
				                    if(w) back4_bfsource |= w->source; 

								  }

							   }
					           break;			   
					         } // end if
	                 }// end step prospect

  		            if(!z)   // not found should never be
						  aig=1;				  
		           } // end index=0

	   			   else  {// it comes from a set, we take it as it is						
						SET chx = zcx.zc[chain4_tsets[x]];
						if(waig){
						    EE.E("set ");chx.Image(this,&EE);EE.Enl("set ");
							Image(back4_bfcount,"back4_bfcount",0);
						}

						int n = chx.ncd; 
						for(int j = 0; j < n; j++) {
							USHORT y = chx.tcd[j] << 1;
							if(waig){
								  EE.E("processing" );zpln.ImageTag(y);
								  EE.Enl( );
								}
							if(y == x)
								continue;
							y ^= 1;
							if((*cum).On(y)){
								back4_bfsource.Set(y);// add to source
								continue; // already false
							}
							if(back4_bfcount.Off(y)) {
								if(waig){
								  EE.E("plus1 itret" );EE.Enl(  itret);
								}
								tret[itret++] = y;
								back4_bfcount.Set(y);
							}
						}// end candidates of the set
					}  //end of the set

				}// end if (i)
//				break; // stop after the step 
			} // end found the tag
		}  // end i
       if(aig || itret>300) {
	       stop_rating=1;
	        if(options.ot) {
				EE.Enl("nested chain go back  invalid situation source");
				EE.E("goback");zpln.ImageTag(tag);
		        EE.E(" npas=");EE.Enl(chain4_npas);
				Image(chain4_cumsteps[0]," start point",0);
				for(int i = 0; i <= chain4_npas; i++) {
					Image(chain4_steps[i],"step",0);
				}

		    }   
	       return 0; // not found, should never be
       }	
   	   itret1++;
	}// end while

	// now prepare the chain in storing mode
	chain4_iresult=0;
    for(int i = 0; i <= chain4_npas; i++) 
		for(int j = itret-1; j >=0; j--)  
			if(chain4_steps[i].On(tret[j])) 
				chain4_result[chain4_iresult++] = tret[j]; 
	if(0   ){  
	  EE.E("nestedgoback end length"); EE.E(chain4_iresult); 
	  EE.E(" itret="); EE.Enl(itret);
	  Image(back4_bfsource,"back4_bf source ",0);
	//  Image(back4_bfcount,"back4_bf count ",0);
	  for(int i=0;i<chain4_iresult;i++){
		  zpln.ImageTag(chain4_result[i]);
		  EE.Esp();
	  }

      EE.Enl();
	}

	return chain4_iresult ; //+ biv; waiting examples for validation
}



/* this is dynamic mode
   and can be a contradiction chain or a dual chain

  still to do to join serate spec
  .2..5...94....9......2..64.2..6....4..8....7.9..5....6.1..2....6..9....2......3..
    [] 1r3c9 ->  ~3r3c9 ->  3r5c9 ->  3r1c8 ->  ~3r5c4 ->  ~3r1c4 ->  3r7c4 ->  ~3r7c1
    [] 1r3c9 ->  ~3r3c9 ->  3r5c9 ->  ~3r5c1 ->  3r7c1

	in that situation serate makes it sorter in that way
	~3r3c9 ->  3r5c9 ->  3r1c8 ->  ~3r5c4 ->  ~3r1c4 ->  3r7c4 
	~3r3c9 ->  3r5c9 ->  ~3r5c1 ->  3r7c1 -> ~3r7c4
	saving 2 in the count
	and a kind of shortcut in once replacing "3r3c9 true" by "1r3c9 false"

	but the shortest should be 
	"3r5c9 false" (save 4) -> 3r3c9 -> ~1r3c9 (add 2) 
	no trick and same count as serate
   */
///

void PUZZLE::NestedForcingLevel4(BFTAG & elims) {
	if(0){  
	    EE.E("\nentry nested forcing level 4 step="); EE.Enl(npas);
	    Image(allsteps,"allsteps",0);
	//    chain4_dpn.Image();
	    EE.Enl("\n");
	}
	for(int i = 2; i < col; i += 2) {
		BFTAG tw(dn.t[i]);
		tw &=tw.Inverse();
		if(tw.IsNotEmpty() || dn.t[i].On(i^1)){
			if(CaseNestedLevel4Case1(i))
			  elims.Set(i ^ 1); 
			else{
				EE.Enl("caseNestedLevel4Case1 invalid return");
				Image(tw," assumed targets",0);
			}
		}
		
        // we look also for fresh dual chains 

		tw=dn.t[i^1];
		if(tw.On(i))  // filter if i^1 false
			tw.SetAll_0();
        tw &= dn.t[i].FalseState();
		tw-=allsteps;
		tw-=elims;   
		if(tw.IsNotEmpty())  {
			if(0){ 
				EE.E("dual chain active source  ");zpln.ImageTag(i);
				Image(tw,"for targets",0);
				EE.Enl();
			}


            USHORT uw[200],iuw;
		    tw.String(uw,iuw);
		     for(int iw=0;iw<iuw;iw++) { // process all targets	
               USHORT j=uw[iw],length=0;
			   int ii=CaseNestedLevel4(i,j);
			   if(!ii)  // should always be	
			      continue;
			   length+=ii;
			   chain4_bf=back4_bfsource;
			   ii=tstore.AddChain(chain4_result,chain4_iresult);
			   int jj=CaseNestedLevel4(i^1,j);
			   if(!jj)  // should always be	
			      continue;
			   length+=jj;
			   chain4_bf|=back4_bfsource;
			   jj=tstore.AddChain(chain4_result,chain4_iresult);
			   USHORT im = tstore.AddMul(ii, jj);
			   tsets[j] =- tcandgo.itt;
				elims.Set(j); 
				tcandgo.tt[tcandgo.itt++].Add(im, chain4_bf,length); 
		    }
		}
	}
}


/*
Now looking for multi chains eliminations if any
the bfs must contain all candidates killed by the main assumption
and the source of all new strong links used

   here again, still some work to meet serate spec (same puzzle as nested forcing)
  .2..5...94....9......2..64.2..6....4..8....7.9..5....6.1..2....6..9....2......3..

[] 8r1c4 ->  ~8r3c6 ->  ~8r3c5 ->  8r3c9 ->  ~1r3c9
[] 8r7c4 ->  ~8r7c1 ->  8r9c1 ->  ~8r7c9 ->  ~8r9c9 ->  8r3c9 ->  ~1r3c9
[] 8r9c4 ->  ~8r9c1 ->  8r7c1 ->  ~8r9c9 ->  ~8r7c9 ->  8r3c9 ->  ~1r3c9

here serate stops at 8r3c9 which is correct
but concludes in once ~1r3c9 same trick as in the forcing chain
saving in total 4 in the count
skfr should save 3 in that situation

*/
void PUZZLE::NestedMultiLevel4(BFTAG & elims) {
   if(0){
	  EE.E("entry nested multi level 4 step ="); EE.Enl(npas);
	  Image((*cum),"cum debut",0);
	  Image(allsteps,"allsteps debut",0);
	 // dn.Image();
   }

   for(int ie = 1; ie < zcx.izc; ie++) {
		const SET &chx = zcx.zc[ie];
		int nni = chx.ncd, aig2 = 0; 
		BFTAG zt;
		zt.SetAll_1();
		zt = zt.FalseState();
		zt -= (*cum);
		if(chx.type - SET_base)
			   continue;
		   

		for(int i = 0; i < nni; i++) {  
			USHORT cd = chx.tcd[i], j = cd << 1; // candidate in tag form
			if(cum->On(j)) {
				aig2 = 1;
				break;
			}// set assigned
			if(dn.t[j].On(j^1)) { // don't do if one candidate cleared through forcing chain
				aig2 = 1;
				break;
			}// set assigned

			zt.Clear(j^1); // set not in the target 			 

			if(cum->Off(j ^ 1)){
				zt &= dn.t[j];	
				zt -= dn.t[j].Inverse(); // don't do if x -> a true 

			}
			
		}
		if(aig2 || zt.IsEmpty() )
			continue;	// if ok second round for action	
		if(0){
			EE.E("N Mul Lvl4 ");
			chx.Image(this,&EE);
			Image(zt," active for ",0);
			EE.Enl();
		}

		USHORT wtt[400],iwtt;
		zt.String(wtt,iwtt);
		for(int ii=0;ii<iwtt;ii++){
		    int i=wtt[ii];
			if(tsets[i])  // don't do if already found
				continue;
			if(0){
				EE.E("gen multi ");chx.Image(this,&EE);
				EE.E(" for ");
				zpln.ImageTag(i);
				EE.Enl();
			}
			chain4_bf.SetAll_0();
			USHORT istored = 0, istoref = 0, tot_count = 0;

			for(int i2=0;i2<nni;i2++) {
				USHORT cd = chx.tcd[i2], j = cd << 1; // candidate in tag form
				if(cum->On(j ^ 1)) {
					chain4_bf.Set(j ^ 1);
					continue;
				}// already false


			   int ii=2;
               if(dpn.t[j].On(i)){ // make it simple if direct
				   chain4_iresult=2;
				   chain4_result[0]=j;
				   chain4_result[1]=i;
			 	   back4_bfsource.SetAll_0();
			   }
			   else
				   ii=CaseNestedLevel4(j,i);
			   if(ii){ // should always be	
				    chain4_bf|=back4_bfsource;
				    if(0){
						EE.E("return from "); zpln.ImageTag(j);
						EE.Esp(); zpln.ImageTag(i);EE.Enl();
						zpln.PrintImply(chain4_result,chain4_iresult);
					}
				    istoref = tstore.AddChain(chain4_result,chain4_iresult);
				    if(!istored)
					   istored = istoref;
				   tot_count += chain4_iresult;			       
				}  //end if

			   else { // stop that case, means faced a contradiction chain
				   if(0){// debugging sequence
					EE.E("\n\nm lvl4 failed for ");
					zpln.ImageTag(j);
					EE.Esp();
					zpln.ImageTag(i);
					chx.Image(this,&EE);
					EE.Enl();
					Image(dpn.t[j],"dpn",j);
					Image(dn.t[j],"dn",j);
					Image((*cum),"cum debut",0);
					Image(allsteps,"allsteps debut",0);
					EE.Enl();
				   }
				   istored=0;
				   break;
			   }
			} // end i2 

			if(istored) { //should always be unless contr chain seen
				USHORT ii = tstore.AddMul(istored, istoref);
				tsets[i] =- tcandgo.itt;
				elims.Set(i); 
				tcandgo.tt[tcandgo.itt++].Add(ii, chain4_bf, tot_count);  
				if(0){					
					EE.Enl("multi created");
					tstore.Print(this,&EE,tcandgo.tt[-tsets[i]].index);
				}
			}			
		} // end tag i
	}// end ie
}

} //namespace skfr