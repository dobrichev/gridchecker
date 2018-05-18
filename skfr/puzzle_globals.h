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

/* included here
            <<<<<<  TCHAIN module   >>>>>>
            <<<<<<  TWO_REGIONS_INDEX   >>>>>>
            <<<<<<  SEARCH_LS_FISH  >>>>>>
            <<<<<<  CELL  CELLS   >>>>>>
            <<<<<<  TPAIRES  >>>>>> 
            <<<<<<  ZGS table   >>>>>>
            <<<<<<  UL_SEARCH module   >>>>>>
            <<<<<< EVENT family classes >>>>>>>>>>
            <<<<< SEARCH_UR >>>>>>>>>>>>>>>>
            <<<<  CANDIDATE    >>>>>
			<<<<< INFERENCES   >>>>>>>
            <<<<<< SETS_BUFFER  SET   SETS    >>>>>>>>>>>>>
			<<<<<<<<<< SQUARE_BFTAG >>>>>>>>>>>>>>>
            <<<<<<<   TCANDGO  >>>>>>>>>>>>>
            <<<<<<< CHAINSTORE   >>>>>>>>>>

/*    <<<<< GG >>>>>>>>>>>>>> 

raw puzzle stored

*/



GG::GG() {	// constructor
	pg = g[0]; 
	pg[81] = 0;
}

int GG::NBlancs() const {
	int i, n = 0;
	for(i = 0; i < 81; i++)
		if(pg[i] == '0')
			n++;   
	return n;
}

int GG::Nfix() const {
	int i, n = 0; 
	for(i = 0; i < 81; i++) 
		if(pg[i] - '0')
			n++;   
	return n;
}

void GG::Image(FLOG * EE, const char * lib) const {
	EE->E(lib); 
	EE->Enl(); 
	char wc[10];
	for(int i=0;i<9;i++) {
		strncpy_s(wc, 10, g[i], 9);
		wc[9] = 0;
		EE->E(i + 1);
		EE->E("=");
		EE->E(wc);
		EE->Enl();
	}
}


/*              <<<<<<  TCHAIN module   >>>>>>
            handling all potential eliminations through chains

			not yet fully thread safe
			global variables still there  
			zpln
			T81t

*/


/*	Upper bound for chains is actually unbounded: the longer chain, the higher rating.
 */

USHORT  steps[] = {4, 6, 8, 12, 16, 24, 32, 48, 64, 96, 128,
            192, 256, 384, 512, 768, 1024, 1536, 2048, 3072, 4096, 6144, 8192};

/* attach the instance to the puzzle
*/

void TCHAIN::SetParent(PUZZLE * parent,FLOG * fl) {
	parentpuz = parent;
	EE=fl;
}
/* start a new cycle (if chains are needed)
*/
void TCHAIN::NewCycle() {
	rating = 200;
	ichain = 0;
	elims_done=0;
	cycle_elims.SetAll_0();
	achieved_rating= parentpuz->ermax;
	maxlength = 10;
}


/* accelerator for the search of loops
   to be revised
*/
void TCHAIN::SetMaxLength(int bw) {
	base = bw;
	if(bw<rating) {
		if(bw<75)
			maxlength = 16; // same as no limit
		else
			maxlength = 100;
	}
	// chexk why, but more creates a problem in bitfield
	else if(bw > 80 || bw == 75)
		maxlength = 100;// linked to CANDGO
	else if(bw == rating)
		maxlength = 4;// equivalent to loop length 6
	else {
		int ii = rating - bw;
		maxlength = steps[ii] / 2 + 2;
	}
}
/* try to clear a candidate should always answer yes
*/

int TCHAIN::ClearChain(int ichain) { // clear candidates
	if(parentpuz->options.ot) {EE->E("clean for UREM=");EE->Enl(chains[ichain].urem);}
	CANDIDATE cw = parentpuz->zpln.zp[chains[ichain].cand];
	return parentpuz->T81t[cw.ig].Change(cw.ch, parentpuz);
}

int TCHAIN::ClearImmediate(USHORT cand){ // clear candidates
	if(cycle_elims.On(cand))
		return 0;
	if(parentpuz->options.ot) {
		EE->Enl("immediate elimination through chain for candidate ");
		parentpuz->zpln.Image(cand);
		EE->Enl();
	}
	cycle_elims.Set(cand);
	CANDIDATE cw = parentpuz->zpln.zp[cand];
	int ir = parentpuz->T81t[cw.ig].Change(cw.ch, parentpuz); // should always answer yes
	if(ir)
		elims_done = 1;   // but keep protected against loops
	return ir;
}

int TCHAIN::Clean() { // clear all pending candidates
	int ir = elims_done;  //  answer is yes is immediate eliminations done
	for(int i = 0; i < ichain; i++)
		ir += ClearChain(i);
	return ir;
}

/* attempt to enter the table of pending eliminations.
   compute the rating and return 0 if
    . higher than current rating, 
	. equal to current rating and 
	   - table is full
	   - or the cand is already in pending eliminations
   if not
   return the rating found
*/
int TCHAIN::GetRatingBase(USHORT bb,USHORT lengthe,USHORT cand) {
	if(lengthe < 2)
		return 0; // should not happen
	USHORT wrating = 300, length = lengthe - 2;
	int ii;
	for(ii=0;ii<22;ii++) 
		if(length<= steps[ii]) {
			wrating = bb + ii;
			break;
		}
	if(wrating>rating) return 0;
	if(wrating == rating) {
		if(ichain >= 30)
			return 0;
		for(int i = 0; i < ichain; i++)
			if(chains[i].cand==cand)
				return 0;
	}
	return wrating;
}


 /* Load after GetRating
    check for safety that the load is valid

	if possible, (rating lower than the achived one)
	do immediate elimination
*/
void TCHAIN::LoadChain(USHORT rat, const char * lib, USHORT cand) {
	if(rat>rating)
		return ;
	if(rat < rating) {
		ichain = 0;
		rating = rat;
	}
	if(rat <= achieved_rating) { //then do it 
		ClearImmediate(cand);
		return;
	}
	if(ichain >= 30) return;
	if(parentpuz->options.ot) {
		EE->Enl();parentpuz->PointK(); EE->Esp(); EE->Enl(lib);
		EE->E(" load tchain rating=");EE->E(rat);
		EE->E(" elimination of ");parentpuz->zpln.Image(cand);
		EE->Enl();
	}
	chains[ichain].cand=cand;
	chains[ichain++].urem=parentpuz->couprem;
}

int TCHAIN::IsOK(USHORT x) {
	// push back an error as soon as possible
	if(parentpuz->stop_rating) return 1;
	if(elims_done) return 1;
	if(!ichain) return 0;
	int ir=((rating <= x) );
	return ir;
}


/*               <<<<<<  TWO_REGIONS_INDEX   >>>>>>

     alternative index for the puzzle
	 only on routine to update the index at the start of a cycle

*/
void TWO_REGIONS_INDEX::Genere(CELL * tw) {
	int i, j;
	for(i = 0; i < 81; i++) {   // on charge tpobit
		const CELL_FIX &w = cellsFixedData[i];
		CELL_VAR x = tw[i].v;
		tpobit.el[w.el].eld[w.pl].Genpo(x);
		tpobit.el[w.pl + 9].eld[w.el].Genpo(x);
		tpobit.el[w.eb + 18].eld[w.pb].Genpo(x);
	}
	// on génère tch a partir de tpo
	for(i = 0; i < 27; i++) {  // les 27 groupes
		for(j = 0; j < 9; j++)
			tchbit.el[i].eld[j].Raz();
		for(j = 0; j < 9; j++)
			for(int k = 0; k < 9; k++)
				if(tpobit.el[i].eld[j].b.On(k))
					tchbit.el[i].eld[k].Set(j);
	}
}

/*
           <<<<<<  SEARCH_LS_FISH  >>>>>>
   class used to search locked sets and fishes

*/

void SEARCH_LS_FISH::SetParent(PUZZLE * parent, FLOG * xx) {
	parentpuz=parent;
	EE=xx;
	regindp=parentpuz->alt_index.tpobit.el;
	regindch=parentpuz->alt_index.tchbit.el;
	regindchcol=&regindch[9];
}

//<<<<<<<<<<<<<<<<<<<<<   On cherche tiroir dans element  // boucle recurrente sur i
int SEARCH_LS_FISH::Tiroir(BF16 fd,int iold,int irang) { // on progresse sur indice et on regarde si fusion n-1  ok
	int i,id=(non.f && (irang-1))? 0:iold+1;  // debut 0 avec pseudos et rang 1
	for (i=id;i<(11-rangc+irang);i++)  // il doit rester des cases
	{ 
		if(eld[i].n<2 ||eld[i].n>rangv) continue;
		if( non.On(i))continue; //pour pseudo
		BF16 wx=eld[i].b|fd; 
		int nx=wx.bitCount();
		if (nx >rangv) continue;
		if(irang==(rangc-2))
		{wf=wx;wi.Set(i);
		if(nx ==rangv)return 1; else return 0; }
		// une erreur a tester    on poursuit si <
		// il peut manquer des tiroirs!!!!! curieux que jamais détecté!!!!
		if(Tiroir(wx,i,irang+1)){wi.Set(i); return 1; }
	}
	return 0;
}

int SEARCH_LS_FISH::Tiroirs(int nn,int hid,int sing) {     //recherche normale des tiroirs
	rangv=nn;single=sing;hid_dir=hid; int ir=0;
	int ied=0,ief=54; 
	if(!hid_dir )ief=27;if(hid_dir==1 )ied=27;if(single){ied=27;ief=54;}
	
	for( e=ied;e<ief;e++)   // direct, hidden or both upon request
	{rangc=rangv; non.f=cases.f=0; if(e<27) e27=e; else e27=e-27;
	if(parentpuz -> NonFixesEl(e%27) < (rangv +1)) continue;

	for(int i=0;i<9-rangv+1;i++)
	{eld=regindp[e].eld;
	int nn= eld[i].n;
	if(nn<2 || nn>rangv) continue;
	BF16 w;w=eld[i].b;
	wi.f=0; wi.Set(i);
	if(!Tiroir(w,i,0)) continue;
	if (UnTiroir())ir= 1;	
	}  
	}
	if(ir)return 1; 
	return 0;
}

int SEARCH_LS_FISH::UnTiroir() {// is there a single required after the locked set to accept it
	int ir=0;
	char ws[10];
	if(single) { // will be covered slowly can be any element row, col, box
		for(int i=0;i<9;i++)  if(wf.Off(i))	{
			USHORT i8 = cellsInGroup[e27][i];
			CELL p=parentpuz->T81t[i8];  if(p.v.typ ) continue;// must be non assigned 
			BF16 wc=p.v.cand-wi;
			for(int j=0;j<9;j++)
				if (wc.On(j) )// a possible hidden digit
					{BF16 wcd=regindch[e27].eld[j].b-wf; // positions still valid
					if(wcd.bitCount()==1)
					{EE->Enl("ecs assignment");
					parentpuz -> FaitGoA(i8,j+'1',4);// stop at first assignment
					ir=1; break;}
			}// end for j if
		}// end hidden ls
		if(!ir) return 0;// no single found
	}// end if single

	else if(e<27) {
		if (!parentpuz ->ChangeSauf(e,wi,wf)&&(!ir))
			return 0;
	}
	else {
		if (!parentpuz ->Keep(e27,wf,wi) &&(!ir))return 0;
	}

	if(!parentpuz->options.ot) return 1; 
	// describe the LS even if no more eliminations after an assignment

	static const char *gt[]={"LS2 ","LS3 ","LS4 ","LS5 " };

	int in=rangv-2,  it2=(e%27)/9,ii2=e%9;
	char c2= (it2-1)?(char)(ii2+'1'):lc[ii2];
	EE->E("->");EE->E(gt[in]); 
	if(e < 27) {
		EE->E(" cells ");	EE->E( wi.String(ws));EE->E(" digits ");
	}
	else {
		EE->E(" digits ");EE->E( wi.String(ws));EE->E(" cells ");
	}
	EE->E(wf.String(ws));EE->E(" ");EE->E(orig[it2]);EE->E(" ");EE->Enl(c2);
	return 1;
}

//<<<<<<<<<<<<<<<<<<<<<   On cherche XW dans lignes ou cols  boucle recurrente sur i
int SEARCH_LS_FISH::XW(BF16 fd,int iold,int irang)  	// en élément i chiffre ch
{   // on progresse sur indice et on regarde si fusion n-1  ok
  for (int i=iold+1;i<9;i++)  // il doit rester des éléments
  { int nn=eld[ch].n;
	 if(nn<2 ||nn>rangv) continue;
    BF16 wfu=regxw[i].eld[ch].b|fd; 
	int nx=wfu.bitCount();
	if (nx >rangv) continue;
    if(irang==(rangv-2)){ if(nx - rangv)continue;
                         wf=wfu; wi.Set(i);return 1; }
      else if(XW(wfu,i,irang+1)) {wi.Set(i); return 1; }
  }
return 0;}
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< en mode XWING liste objets et chiffres
int SEARCH_LS_FISH::GroupeObjetsChange(int dobj,int ch)
{BF16 x(ch); int ir=0;
 for(int i=0;i<9;i++)   if(wf.On(i)) ir+= parentpuz ->ChangeSauf(dobj+i,wi,x);
return ir;}
//<<<<<<<<<<<<<<<<<<<<<<<< ici chiffre en majeur éléments en mineur
int SEARCH_LS_FISH::XW(int nn)
{char ws[10];
 static const char *gxw[]={"XWing ","SwordFish  ","Jelly (XW4) ","Squid (XW5)  " ,
              "Whale (XW6)","Leviathan (XW7)" };
 BF16 w; rangc=nn;
 for(int i=0;i<9;i++)
  {ch=(UCHAR)i;
   for(int iel=0;iel<10-rangv;iel++)
   { regxw=regindch;
	 eld=regxw[iel].eld;     
   	 int nn=eld[i].n;
    if(nn>1 &&nn<=rangv)
    { w=eld[i].b; wi.f=0; wi.Set(iel);
      if( XW(w,iel,0) )
       { if(GroupeObjetsChange(9,i) )  // action colonnes
	     {EE->E(gxw[rangv-2]);	 EE->E(" digit ");     EE->E(i+1); 
	      EE->E( " columns ");   EE->E(wf.String(ws));
		  EE->E(" rows ");  EE->Enl(wi.String(ws));     
		  return 1; 	 }}
       }
	 regxw=regindchcol;
	 eld=regxw[iel].eld;
     nn=eld[i].n;
     if(nn<2 || nn>rangv) continue;
     w=eld[i].b;  	wi.f=0; wi.Set(iel);
     if( XW(w,iel,0) )
      { if(GroupeObjetsChange(0,i) ) // action lignes
	   {EE->E(gxw[rangv-2]);	 EE->E(" digit ");
	    EE->E(i+1); EE->E(" rows ");	EE->E(wf.String(ws));
		EE->E( " columns ");	  EE->Enl(wi.String(ws));     
		return 1; }  	 }
    } // end iel
  }    // end i niv
return 0;}



/*               <<<<<<  CELL  CELLS   >>>>>>

     main table for the cells of the  puzzle
	 this is the entry point for any elimination
	 keep updated situation of candidates and assigned/given

	 a copy of the table is needed in some steps as UR search


*/
int CELL::Change(int ch,PUZZLE * ppuz) {
	if(v.cand.Off(ch))
		return 0;
	if(ppuz->CheckChange(f->i8, ch))
		return 0;
	v.cand.Clear(ch);
	v.ncand = v.cand.CountEtString(scand);
	ppuz->c[ch].Clear(f->i8);
	return 1;
}

// obsolete to clean later
int CELL::Changex(PUZZLE &puz, int ch) {
	if(v.cand.Off(ch))
		return 0;
	if(puz.CheckChange(f->i8, ch))
		return 0;
	v.cand.Clear(ch);
	v.ncand = v.cand.CountEtString(scand);
	puz.c[ch].Clear(f->i8);
	return 1;
}


int CELL::Change (BF16 cb9,PUZZLE * ppuz) {    // clear candidates 
	   int ir=0;
	   for(int i=0; i < 9; i++) 
		   if(cb9.On(i))
			   ir += Change(i,ppuz);
	   return ir;
   }


// obsolete to clean later

int CELL::Changey (PUZZLE &puz, BF16 cb9) {    // clear candidates 
	   int ir=0;
	   for(int i=0; i < 9; i++) 
		   if(cb9.On(i))
			   ir += Changex(puz, i);
	   return ir;
   }


int CELL::Keep (BF16 cb9, PUZZLE * ppuz) {       // clear candidates others than
	   int ir=0;
	   for(int i = 0; i < 9; i++) {
		   if(v.cand.On(i) && !cb9.On(i)) {
			   Change(i,ppuz);
			   ir=1;
		   }
	   }
	   return ir;
   }

int CELL::Keep(int ch1,int ch2,PUZZLE * ppuz) { // clear candidates others than
	   int ir = 0;
	   for(int i = 0; i < 9; i++) {
		   if(v.cand.On(i) && (i - ch1) && (i - ch2)) {
			   Change(i,ppuz);
			   ir = 1;
		   }
	   }
	   return ir;
   }

int CELL::Keepy(PUZZLE &puz, BF16 cb9) {       // clear candidates others than
	   int ir=0;
	   for(int i = 0; i < 9; i++) {
		   if(v.cand.On(i) && !cb9.On(i)) {
			   Changex(puz, i);
			   ir=1;
		   }
	   }
	   return ir;
   }

int CELL::Keepy(PUZZLE &puz, int ch1,int ch2) { // clear candidates others than
	   int ir = 0;
	   for(int i = 0; i < 9; i++) {
		   if(v.cand.On(i) && (i - ch1) && (i - ch2)) {
			   Changex(puz, i);
			   ir = 1;
		   }
	   }
	   return ir;
   }

  void CELL::Fixer(UCHAR type,UCHAR ch) { // force digit as valid in the solution
	   v.Fixer(type, ch);
	   scand[1] = 0;
	   scand[0] = '1' + ch;
   }


void CELLS::SetParent(PUZZLE * parent,FLOG * fl){
 parentpuz=parent;
 EE=fl;}


void CELLS::init() {
	for(int i = 0; i < 81; i++) {
		t81[i].v.Init();
		t81[i].f = &cellsFixedData[i];
	}
}
void CELLS::Fixer(int ch, int i8, UCHAR typ) {
	t81[i8].Fixer(typ, ch);
	parentpuz->cFixer(ch, i8);
}

int CELLS::Clear(BF81 &z, int ch) {
	//EE->E("clear tCELL ");EE->E(ch+1);z.ImagePoints();  EE->Enl();
	int ir = 0;
	for(int i = 0; i < 81; i++)
		if(z.On(i))
			ir += t81[i].Change(ch,parentpuz);
	return ir;
}
int CELLS::Clear(BF81 &z, BF16 x) {
	int ir = 0;
	for(int j = 0; j < 9; j++)
		if(x.On(j))
			ir += Clear(z,j);
	return ir;
}
//<<<<<<<<<<<<<<<<<<<<    specific ot UR/UL filter to find the lowest length
int CELLS::CheckClear(BF81 &z, BF16 x) {
	for(int i = 0; i < 81; i++)
		if(z.On(i))
			if((t81[i].v.cand&x).f)
				return 1;
	return 0;
}// positive as soon as one effect found

//<<<<<<<<<<<<<<<<<<<<<<<<<
void CELLS::Actifs(BF81 & z) {
	z.SetAll_0();
	for(int i = 0; i < 81; i++)
		if(!t81[i].v.typ)
			z.Set(i);
}
//<<<<<<<<<<<<<<<<<
BF16 CELLS::GenCand(BF81 & z) {
	BF16 w;
	w.f = 0;
	for(int i = 0; i < 81; i++)
		if(z.On(i) && (!t81[i].v.typ))
			w = w | t81[i].v.cand;
	return w;
}
//<<<<<<<<<<<<<<<<<     y compris assigned pour RIs
BF16 CELLS::GenCandTyp01(BF81 & z) {
	BF16 w;
	w.f = 0;
	for(int i = 0; i < 81; i++)
		if(z.On(i) && t81[i].v.typ < 2)
			w = w | t81[i].v.cand;
	return w;
}
/*
//<<<<<<<<<<<<<<<<
void 	 CELLS::GenzCand(BF81 & z1,BF81 & z2,int ic)
{z2.Init();  for(int i=0;i<81;i++)
if(z1.On(i)&&(!t81[i].v.typ)&&t81[i].v.cand.On(ic)) z2.Set(i);  }
*/
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
void CELLS::Candidats() {
	if(!parentpuz->options.ot)
		return; 
	int i, j, l, lcol[9], tcol = 0;
	char * pw;       //lcol  largeur maxi colonne
	EE->Enl("PM map ");  
	for(i = 0; i < 9; i++) {  // attention ici i indice colonne
		lcol[i] = 2;    // 2  mini tous chiffres imposés
		for(j = 0; j < 9; j++) {
			l = (int)strlen(t81[9 * j + i].strcol()); 
			if(l > lcol[i])
				lcol[i] = l;
		}
		tcol += lcol[i];
	}
	EE->Enl();
	for(i = 0; i < 9; i++) {
		if((i == 3) || (i == 6))
			EE->E("|");
		EE->E((char)('A' + i));
		EE->E(Blancs(lcol[i], 1));
	} 
	EE->Enl();
	for(i = 0; i < 9; i++) { // maintenant indice ligne
		if((i == 3) || (i == 6)) {
			for(int ix = 0; ix < (tcol + 10); ix++)
				EE->E((char)'-');
			EE->Enl();
		}
		for(j = 0; j < 9; j++) {
			if((j == 3) ||(j == 6))
				EE->E("|");
			CELL * pp8 = &t81[9*i + j];
			pw = pp8->strcol();		  
			EE->E(pw);
			EE->E(Blancs(lcol[j] + 1 - (int)strlen(pw), 1));
		} // end for j
		EE->Enl();
	} // end for i
	EE->Enl("\n\n");
}

int CELLS::RIN(int aig) {      // look for unique rectangle 
	int ir=0;
	parentpuz->urt.Init();
	for(int i0 = 0; i0 < 3; i0++) // band/stack 1 to 3
		for(int i1 = 0; i1 < 2; i1++)
			for(int i2 = i1 + 1; i2 < 3; i2++) // 2 rows  
				for(int j1 = 0; j1 < 2; j1++)
					for(int j2 = j1 + 1; j2 < 3; j2++) // boxes   12 13 23
						for(int j3 = 0; j3 < 3; j3++)
							for(int j4 = 0; j4 < 3; j4++) {  // 2 cols  
								//analysing band and stack, main diagonal symmetry
								int l1 = 3 * i0 + i1, l2 = 3 * i0 + i2, 
									c1 = 3 * j1 + j3, c2 = 3 * j2 + j4;
								if(parentpuz->ur.RIDx(l1, l2, c1, c2) || 
								   parentpuz->ur.RIDx(c1, c2, l1, l2))
									ir++;
							}
	return ir;
}


/*
           <<<<<<  TPAIRES  >>>>>>
   class used for all processes based on pairs

   BUGS  XYwing  XYZWing and partly ULs

*/

void TPAIRES::SetParent(PUZZLE * parent, FLOG * xx) {
	parentpuz = parent;
	EE = xx;
}

void TPAIRES::CreerTable(const CELL * tt) {
	ip = 0;
	ntplus = aigpmax = 0;
	zplus.SetAll_0();
	for(int i = 0; i < 81; i++) {
		int n = tt[i].v.ncand;
		if(n == 2) {
			zp[ip].Charge(tt[i]);
			zpt[ip] = zp[ip++];
		}
		else if(n > 2) {
			if(ntplus < 8) {
				tplus[ntplus] = i;//direct access to first plus cells
				ntplus++;
				zplus.Set(i);
			}
			if(n > aigpmax)
				aigpmax = n;
		}
	}
	// now sorting the table zpt for the search of UR/UL 
	for(int i = 0; i < ip - 1; i++) {
		for(int j = i + 1; j < ip; j++) {
			if(zpt[j].pa.f<zpt[i].pa.f ||(zpt[j].pa.f == zpt[i].pa.f && zpt[j].i8<zpt[i].i8)) {
				PAIRES temp = zpt[i];
				zpt[i] = zpt[j];
				zpt[j] = temp;
			}
		}
	}
	// and final entries in tp izpd izpf
	np = 0;
	if(!ip)
		return;
	tp[0] = zpt[0].pa;
	izpd[0] = 0;
	for(int i = 1; i < ip; i++) {
		if(zpt[i].pa.f == tp[np].f)
			continue;
		izpd[++np] = i;
		tp[np] = zpt[i].pa;
	}
	izpd[++np] = ip;
}


//====================================
int TPAIRES::UL() {
	int ir = 0;
	parentpuz->tult.Init();
	for(int i = 0; i < np; i++) {
		USHORT id = izpd[i], ie = izpd[i + 1];
		// EE->Enl("un depart paire");
		UL_SEARCH uls(tp[i], this, &zpt[id], ie - id,parentpuz,EE); //build search 
		for(int j = id; j < ie - 1; j++) {
			for(int k = j + 1; k < ie; k++) {
				USHORT i1 = zpt[j].i8, i2 = zpt[k].i8;
				const CELL_FIX &p1 = cellsFixedData[i1], &p2 = cellsFixedData[i2];
				if(!(p1.el==p2.el || p1.pl==p2.pl))
					continue; // start row or col
				//  EE->E(p1.pt); EE->E(p2.pt);EE->Enl("un depart lig/col");
				UL_SEARCH ulsn(&uls);
				ulsn.Set(i1);
				if(p1.el == p2.el)
					ulsn.el_used.Set(p1.el);  
				else
					ulsn.el_used.Set(p1.pl+9); 
				ir += ulsn.Add_Chain(i2);  // looking for all equivalent moves
			}// end k
		} // end j
	}// end i
	return ir;
}

/* searching and processing all forms of bugs included in SE
   type 1 one extra cell  5.6
   type 2 2 extra cells with one same extra digit  5.7
   type 3 extra cells combined to a naked duo  5.8
   type 3 extra cells combined to a naked triplet 5.9
   type 3 extra cells combined to a naked quad 6.0 
   type 3 extra cells combined to a naked (5) 6.1
   type 4 extra cells (2) have a locked digit 5.7   */


//==============================
int TPAIRES::BUG() {
	EE->Enl("debut recherche bug");
	aigun = 0;
	if(ntplus > 6 || aigpmax > 4)
		return 0;  // maxi a vérifier 6 cases et 4 candidats
	// set the parity of digits for bivalue cells in all elements
	for(int i = 0; i < 27; i++)
		el_par_ch[i].f = 0;
	for(int i = 0; i < ip; i++) {
		CELL p = parentpuz->T81t[zp[i].i8];  
		el_par_ch[p.f->el] ^= p.v.cand; 
		el_par_ch[p.f->pl+9] ^= p.v.cand; 
		el_par_ch[p.f->eb+18] ^= p.v.cand;   
	}
	int aigspecial=0;
	//  one special case for 3 cells plus in the same in "L" 
	if(ntplus>2){
		for(int iel=0;iel<27;iel++){
			BF81 ww=zplus-cellsInHouseBM[iel];
			if(ww.IsNotEmpty()) continue;  // all plus in the same box
			if(iel<18) break; //standard if all row or column

			if(ntplus>3) return 0; // does not work for more

			// if not "L", it is standard
			int corner=-1;
			for(int i=0;i<3;i++){
				CELL p = parentpuz->T81t[tplus[i]];
				BF81 wrow(cellsInHouseBM[p.f->el]),wcol(cellsInHouseBM[p.f->pl+9]);
				int c1=(wrow&zplus).Count(),c2=(wcol&zplus).Count();
				if(c1==2 && c2==2 ) 	{corner=i;			break;}
			}

			if(corner<0) break; // Defined in row or col, standard
			aigspecial=1;

			BF16 wpar=el_par_ch[iel]; // wil be adjusted by wings
			// now solve the 2 wings of the "L"
			for(int i=1;i<3;i++){
				int ii=(corner+i)%3; // the tplus index
				CELL px = parentpuz->T81t[tplus[ii]];  
				BF81 wr(cellsInHouseBM[px.f->el]);
				int elx=((wr&zplus).Count()==1)?px.f->el:px.f->pl+9;
				BF16 wc = px.v.cand & el_par_ch[elx],
					 w = px.v.cand - wc;
				if(wc.bitCount() - 2)return 0;  // must be 2 everywhere
				tplus_par[ii]=w;
				tplus_keep[ii]=wc;
				wpar ^= wc; // adjust parity in the box
			}
			// and solve now the corner in 
			CELL pcorn = parentpuz->T81t[tplus[corner]];  
			BF16 wc_corn = pcorn.v.cand & wpar,
				 w_corn = pcorn.v.cand - wc_corn;
			if(wc_corn.bitCount() - 2)return 0;  // must be 2 as usual
			tplus_par[corner]=w_corn;
			tplus_keep[corner]=wc_corn;

		}
	}

	if(!aigspecial){
		// check one and only one parity complement par cell with "plus"
		for(int i = 0; i < ntplus; i++) {
			tplus_par[i].f=1024;
			CELL p = parentpuz->T81t[tplus[i]];  
			USHORT t[3];
			t[0]=p.f->el; t[1]=p.f->pl+9;t[2]=p.f->eb+18;
			for(int j=0;j<3;j++){
				BF81 ww(cellsInHouseBM[t[j]]);
				if((ww & zplus).Count()-1) continue;
				BF16 wc = p.v.cand & el_par_ch[t[j]],
					 w = p.v.cand - wc;
				if(wc.bitCount() - 2)return 0;  // must be 2 everywhere

				if(tplus_par[i].f-1024){
					if(tplus_par[i].f-w.f) return 0;
				}
				else{
					tplus_par[i]=w;
					tplus_keep[i]=wc;
				}	
			}
			if(tplus_par[i].f==1024) return 0; // not find possibility to affect
		}
	}
	// check always  that each region is parity 0
	if(ntplus>1){
		for(int iel = 0; iel < 27; iel++) {
			BF81 ww(cellsInHouseBM[iel]);
			if((ww&zplus).Count()<2) continue; // only multiple plus
			BF16 wd=el_par_ch[iel];
			for(int i = 0; i < ntplus; i++) {
				if(ww.On(tplus[i]))
					wd ^= tplus_keep[i];
			}
			if(wd.f) return 0;
		}
	}

	if(ntplus == 1){
		parentpuz->T81t[tplus[0]].Keepy(*parentpuz,tplus_par[0]); // eliminate the others
		BugMess(" 1");
		return 1;
	}


	// set the parity of digits for bivalue cells in all elements
//	for(int i = 0; i < 27; i++)
//		el_par_ch[i].f = 0;
//	for(int i = 0; i < ip; i++) {
//		CELL p = parentpuz->T81t[zp[i].i8];  
//		el_par_ch[p.f->el] ^= p.v.cand; 
//		el_par_ch[p.f->pl+9] ^= p.v.cand; 
//		el_par_ch[p.f->eb+18] ^= p.v.cand;         }
//	if(ntplus == 1)
//		return Bug1();
	if(Bug2())
		return 1; // priority to bug 2
	if(Bug3a(58))
		return 1;
	return 0;
}
//===================== calls 
int TPAIRES::Bug3a(int rat) {
	brat = rat;  // maximum authorized in that step
	for(int i = 0; i < 27; i++) {
		//if(zplus.EstDans(divf.elz81[i])) {
		if(zplus.EstDans(cellsInHouseBM[i])) {
			ntpa = nwp = 0;// collect data in that row/col/box
			candp_or.f = candnp_or.f = nwp = 0;
			//candp_xor.f = 0;
			//candnp_xor.f = 0;
			for(int j = 0; j < 9; j++) {
				//int i8 = divf.el81[i][j];
				int i8 = cellsInGroup[i][j];
				CELL p = parentpuz->T81t[i8];
				if(zplus.On(i8)) { // wplus has the same order as tplus
					candnp_or |= p.v.cand;
					//candnp_xor ^= p.v.cand;
					//wplus[nwp] = p.v.cand;
					nwp++;
				}
				else if(p.v.ncand == 2) {
					candp_or |= p.v.cand;
					//candp_xor ^= p.v.cand;
					tpa[ntpa++] = i8;
				}
			}
			aigun = 1;
			if(Bug3(i))
				return 1;//bug3 and bug4
			continue;//only one row/col/box
		}
	}
	return 0;
}


//================================ cells in different objects or one digit
int TPAIRES::Bug2() { // any number of cells, but 6 seems very high
// revised code as of april 2012
	if(tplus_par[0].bitCount() -1) return 0; // must be one digit
	BF81 zw(cellsFixedData[tplus[0]].z);
	for(int i=1;i<ntplus;i++){
		if(tplus_par[0].f - tplus_par[i].f) return 0; // same digit everywhere
		zw &= cellsFixedData[tplus[i]].z;
	}
	if(zw.IsEmpty()) return 0; // must have a comon control on some cells

	// ok for bug type 2 clear the commonly controled  cells
	int ir = 0, ch = tplus_par[0].First(); 
	for(int i = 0; i < 81; i++)
		if(zw.On(i))
			ir += parentpuz->T81t[i].Changex(*parentpuz, ch);
	if(ir) {
		BugMess("2 same digit");
		parentpuz->SetDif(57);
	}
	return ir;
}
//===========================================
void TPAIRES::BugMess(const char * lib) const {
	EE->E("Bug type ");
	EE->Enl(lib);
	if(parentpuz->options.ot)
		parentpuz->T81C->Candidats();
}

//===================  all cells in the same  element(s )(can be type 2)
int TPAIRES::Bug3(int el) {
	// update new code as of april 2012
	if((ntplus == 2) && Bug_lock(el))
		return 1;
	BF16 welim;
	for(int i = 0; i < ntplus; i++) {
		welim|=tplus_par[i];
	}	 
	return Nacked_Go(welim);
}

int TPAIRES::Nacked_Go(BF16 welim) {
	char ws[10];
	//we look now for  "naked locked sets"
	EE->E("recherche  bug3_4 Nacked ok to go welim= ");
	EE->Enl( welim.String(ws));
	int nelim = welim.bitCount(); // look for naked in increasing order
	if(nelim < 2 || nelim > 5)
		return 0;
	if(brat == 58 && nelim < 3) { //then search for naked 2
		for(int i = 0; i < ntpa; i++) {
			if(parentpuz->T81t[tpa[i]].v.cand.f == welim.f) { // we found it
				int ir = 0;
				for(int j = 0; j < ntpa; j++)
					if(j-i)
						ir += parentpuz->T81t[tpa[j]].Change(welim,parentpuz);
				if(ir) {
					BugMess("type 3/4 naked pair");
					parentpuz->SetDif(58);
					return 1;
				}
			}
		}
	}
	if(brat == 59) {        // look for triplet
		for(int i1 = 0; i1 < ntpa - 1; i1++) {
			for(int i2 = i1 + 1; i2 < ntpa; i2++) {
				BF16 ww = welim | parentpuz->T81t[tpa[i1]].v.cand | parentpuz->T81t[tpa[i2]].v.cand;
				if(ww.bitCount() - 3)
					continue; // if not we got it
				int ir = 0;
				for(int j = 0; j < ntpa; j++)
					if((j - i1) && (j - i2))
						ir += parentpuz->T81t[tpa[j]].Change(ww,parentpuz);
				if(ir) {
					BugMess("type 3/4 naked triplet");
					parentpuz->SetDif(59);
					return 1;
				}
			}
		}
	} // end triplet
	if(brat == 60) {                   // look for quad
		for(int i1 = 0; i1 < ntpa - 2; i1++) {
			for(int i2 = i1 + 1; i2 < ntpa - 1; i2++) {
				for(int i3 = i2 + 1; i3 < ntpa; i3++) {
					BF16 ww = welim | parentpuz->T81t[tpa[i1]].v.cand | parentpuz->T81t[tpa[i2]].v.cand | parentpuz->T81t[tpa[i3]].v.cand;
					if(ww.bitCount() - 4)
						continue; // if not we got it
					int ir = 0;
					for(int j = 0; j < ntpa; j++)
						if((j - i1) && (j - i2) && (j-i3))
							ir += parentpuz->T81t[tpa[j]].Change(ww,parentpuz);
					if(ir) {
						BugMess("type 3/4 naked quad");
						parentpuz->SetDif(60);
						return 1;
					}
				}
			}
		}
	}// end quad
	if(brat == 61) {                   // look for (5)
		for(int i1 = 0; i1 < ntpa - 3; i1++) {
			for(int i2 = i1 + 1; i2 < ntpa - 2; i2++) {
				for(int i3 = i2 + 1; i3 < ntpa - 1; i3++) {
					for(int i4 = i3 + 1; i4 < ntpa; i4++) {
						BF16 ww = welim | parentpuz->T81t[tpa[i1]].v.cand | parentpuz->T81t[tpa[i2]].v.cand
							| parentpuz->T81t[tpa[i3]].v.cand | parentpuz->T81t[tpa[i4]].v.cand;
						if(ww.bitCount() - 5)
							continue; // if not we got it
						int ir = 0;
						for(int j = 0; j < ntpa; j++)
							if((j - i1) && (j - i2) && (j - i3) && (j - i4))
								ir += parentpuz->T81t[tpa[j]].Change(ww,parentpuz);
						if(ir) {
							BugMess("type 3/4 naked (5)");
							parentpuz->SetDif(61);
							return 1;
						}
					}
				}
			}
		}
	}// end (5)
	return 0;
}

//===================  all cells in the same  element(s )(can be type 2)
int TPAIRES::Bug_lock(int el) {
	char ws[10];
	EE->Enl("recherche  bug_lock"); 
	BF16 clock = candnp_or - candp_or;  // locked candidate
	EE->E(" clock=");
	EE->Enl(clock.String(ws));
	if(!clock.f)
		return 0; 
	CELL p1 = parentpuz->T81t[tplus[0]], p2 = parentpuz->T81t[tplus[1]];
	int el1 = p1.f->el, el2 = p2.f->el; // use the row in priority
	if(el < 9)  {
		el1 = p1.f->pl + 9;
		el2 = p2.f->pl + 9;
	}  // and col if row is "el"
	if(el1==el2) {
		if(el>=9) return 0;
		el1 = p1.f->pl + 9;  // can not accept el1=el2 chaeck the other direction
		el2 = p2.f->pl + 9;
	}

	BF16 wc1 = (p1.v.cand & el_par_ch[el1]), wce1 = p1.v.cand - wc1,
		wc2 = (p2.v.cand & el_par_ch[el2]), wce2 = p2.v.cand - wc2;
	EE->E(p1.f->pt);
	EE->E(" el=");
	EE->E(el1 + 1);
	EE->E(" wc1=");
	EE->Enl(wc1.String(ws));
	EE->E(p2.f->pt);
	EE->E(" el=");
	EE->E(el2 + 1);
	EE->E(" wc2=");
	EE->Enl(wc2.String(ws));

	// modified 2011 12 23 see puzzle
	// .....1.23....2.456...6..78...8..3..5.4.....3.6..2..1...27..4...893.7....46.8.....
	//if(wce2.f-wce1.f) return 0; // must be the same digit

	if((wc1.bitCount() - 2) || (wc2.bitCount() - 2))
		return 0;	 

	parentpuz->T81t[tplus[0]].Keepy(*parentpuz, wce1 | clock);
	parentpuz->T81t[tplus[1]].Keepy(*parentpuz, wce2 | clock);
	BugMess("3/4 a digit locked");
	parentpuz->SetDif(57);
	return 1;
}



//<<<<<<<<<<<<<<<<<<<< // depart deux paires pas objet commun et trio
int TPAIRES::XYWing() { // troisieme par les isoles  objets  communs
	int ir = 0;
	for(int i = 0; i < ip - 1; i++) {
		for(int j = i + 1; j < ip; j++) {
			if(CommunPaires(i, j))
				continue;
			BF16 w = zp[i].pa|zp[j].pa;
			if(w.bitCount() - 3)
				continue;
			BF16 w1 = (zp[i].pa&zp[j].pa), w2=w1 ^ w;  // les deux non communs
			for(int k = 0; k < ip; k++) {
				if(zp[k].pa.f - w2.f)
					continue; // il faut =
				if(!CommunPaires(i, k))
					continue;
				if(!CommunPaires(j, k))
					continue;
				// on a un XYWing  potentiel
				int ich = w1.First(); // le chiffre 
				//BF81 z1 = t81f[zp[i].i8].z & t81f[zp[j].i8].z,
				BF81 z1 = cellsFixedData[zp[i].i8].z;
				z1 &= cellsFixedData[zp[j].i8].z;
				BF81 z2 = z1 & parentpuz->c[ich];  // z2 est à supprimer
				if(z2.IsNotEmpty()) {
					if(parentpuz->options.ot)
						CommunLib(i, j, zp[k].i8, "->XY WING pivot= ");
					parentpuz->T81->Clear(z2, ich);
					return 1;
				}
			} 
		}
	}
	return ir;
}

//<<<<<<<<<<<<<<<<<<<< // depart deux paires pas objet commun et trio
int TPAIRES::XYZWing() { // troisieme est le trio objets communs
	int ir = 0;  
	for(int i = 0; i < ip - 1; i++) {
		for(int j = i + 1; j < ip; j++) {
			if(CommunPaires(i, j))
				continue;
			BF16 w = zp[i].pa|zp[j].pa;
			if(w.bitCount() - 3)
				continue;
			BF16 w1 = (zp[i].pa&zp[j].pa);  // le chiffre
			for(int k = 0; k < 81; k++) {
				if(parentpuz->T81t[k].v.cand.f - w.f)
					continue; // il faut = trio
				if(!CommunTrio(k, i)) continue;
				if(!CommunTrio(k, j)) continue;
				// on a un XYZWing  potentiel
				int ich = w1.First(); // le chiffre
				//BF81 z1 = t81f[zp[i].i8].z & t81f[zp[j].i8].z & t81f[k].z;
				BF81 z1 = cellsFixedData[zp[i].i8].z;
				z1 &= cellsFixedData[zp[j].i8].z;
				z1 &= cellsFixedData[k].z;
				BF81 z2 = z1 & parentpuz->c[ich];  // z2 est à supprimer
				if(z2.IsNotEmpty()) {
					if(parentpuz->options.ot)
						CommunLib(i, j, k, "->XYZ WING pivot= ");
					parentpuz->T81->Clear(z2, ich);
					return 1;
				}
			}   
		}
	}
	return ir;
}

void TPAIRES::CommunLib(int i, int j, int k, const char * lib) {
	if(!parentpuz->options.ot)
		return;
	EE->E(lib);
	EE->E(cellsFixedData[k].pt);
	EE->E(" ");
	EE->E(parentpuz->T81t[k].scand);	 
	EE->E(" (1)=");
	EE->E(cellsFixedData[zp[i].i8].pt);
	EE->E(" ");
	EE->E(parentpuz->T81t[zp[i].i8].scand);
	EE->E(" (2)=");
	EE->E(cellsFixedData[zp[j].i8].pt);
	EE->E(" ");
	EE->Enl(parentpuz->T81t[zp[j].i8].scand);	
}

int TPAIRES::CommunPaires(int i, int j) {
	return parentpuz->T81t[zp[i].i8].ObjCommun(&parentpuz->T81t[zp[j].i8]);
}
int TPAIRES::CommunTrio(int i, int j) {
	return parentpuz->T81t[i].ObjCommun(&parentpuz->T81t[zp[j].i8]);
}

/*              <<<<<<  ZGS table   >>>>>>
        list of all BF81 in use
		can disappear in an optimised code for skfr
		inherited from GP solver where more pattern were used
		as it is, it is a a "quasi constant"
		initialised by the constructor

		81 BF81 for the cells
		then 54 BF81 for the groups row/box column box

*/


ZGROUPE::ZGROUPE () {
	int i,j;
	BF81 z0,z1; 
	z0.SetAll_0(); 
	z1.SetAll_1();
	for(i=0;i<81;i++)
	{
		z[i]=z0;
		z[i].Set(i);
	}  // les z de 81 cases
	int k,n=0,il=81;                    // puis les z de groupe
	for(i=0;i<18;i++)
		for(j=0;j<3;j++)
		{
			z[il].SetAll_0();
			if(i<9)
				for(k=0;k<3;k++)
					z[il].Set(n++);
			else
			{
				int c=i-9; 
				for(k=0;k<3;k++)
					z[il].Set(c+27*j+9*k);
			}
		il++; 
		}
	//ReInit();
}



/*             <<<<<<  UL_SEARCH module   >>>>>>

that clas is a support to find a UL
each occurence is normally stored for further processins


*/



// constructor to start a UL search

UL_SEARCH::UL_SEARCH(BF16 c, TPAIRES * tpae, PAIRES * pae, USHORT npae,
					 PUZZLE * parent, FLOG * xx ) {
	 // initial for a new start
	 parentpuz = parent;
	 EE=xx;
	 tpa = tpae;
	 pa = pae;
	 npa = npae;
	 chd = cht = c;
	 nadds = line_count = 0; 
	 char  st[10];
	 c.String(st);
	 c1 = st[0] - '1';
	 c2 = st[1] - '1';
	 cells.SetAll_0();
	 el_used.f = parity.f = 0;
	 for(int i = 0; i < 27; i++)
		 elcpt[i] = 0;
}


//========================= insert a new cell after el_used correct
void UL_SEARCH::Set(int i8) { // el_used already ok if necessary
	cells.Set(i8);  
	const CELL_FIX &p = cellsFixedData[i8];
	last = i8; 
	CELL_VAR pv = parentpuz->T81tc[i8].v;
	parity.Inv(p.el);
	parity.Inv(p.pl + 9);
	parity.Inv(p.eb + 18);                   
	elcpt[p.el]++;
	elcpt[p.pl + 9]++;
	elcpt[p.eb + 18]++;
	cht |= pv.cand;
	int nc = pv.cand.bitCount(); 
	if(nc > 2)
		adds[nadds++] = i8;
	// EE->E("UL lc=");EE->E(line_count);EE->Esp();EE->Enl(p.pt);
	tcount[line_count++] = i8;
}

//====================================
// Check if the loop has more than 0 solutions by parity
bool UL_SEARCH::ParityCheck(void) {
	// Check for the "0 solutions" parity check
	unsigned int oddPos = 0, evenPos = 0;
	bool isOdd = false;
	for(int i = 0; i < (line_count - 1); ++i) {
		isOdd = !isOdd;
		unsigned int curRow = tcount[i]/9;
		unsigned int curCol = tcount[i] % 9;
		unsigned int curBox = I81::Boite(curRow, curCol);
		int newPos = 1 << curRow | 1 << (curCol + 9) | 1 << (curBox + 18);
		if(isOdd) {
			if(oddPos & newPos)
				return false;
			oddPos |= newPos;
		}
		else {
			if(evenPos & newPos)
				return false;
			evenPos |= newPos;
		}
	}
	if(oddPos != evenPos)
		return false;
	return true;
}


//============================= recurrent process to add next point
int UL_SEARCH::Add_Chain(int i8) {
	if(line_count > 20)
		return 0; // securite
	if(cells.On(i8)) { // terminé on élimine URs et fausses boucles
		//EE->E("UL end ");EE->E(cellsFixedData[i8].pt);EE->Esp();cells.ImagePoints();EE->Enl();
		if(line_count < 5 || (i8 - tcount[0]))
			return 0;  
		tcount[line_count++] = i8;
		return Loop_OK();
	}
	Set(i8);              // On met le point en place
	const CELL_FIX &f = cellsFixedData[i8];

	// a défaut une case avec additifs  ligne, puis col, puis bloc en paire
	// uniquement dans éléments non traités et si pas de double paire
	// not more than 3 adds except one digit
	if(nadds > 7 || (cht.bitCount() > 3 && nadds > 2))
		return 0; 
	if(El_Suite(f.el))
		return 1;
	if(El_Suite(f.pl + 9))
		return 1;
	if(El_Suite(f.eb + 18))
		return 1;
	return 0;
}

//====================================
int UL_SEARCH::El_Suite(USHORT ele) {
	if(el_used.On(ele))
		return 0;
	//EE->E("suite el=");EE->Enl(ele+1);
	BF16 wc = parentpuz->alt_index.tchbit.el[ele].eld[c1].b & parentpuz->alt_index.tchbit.el[ele].eld[c2].b;
	for(int i = 0; i < 9; i++) {
		if(wc.On(i)) { // cells with both digits
			int i8r = cellsInGroup[ele][i];
			//EE->E("essai i8=");EE->Enl(cellsFixedData[i8r].pt);
			if(ele > 17) { // in a box, only not row col
				const CELL_FIX &f = cellsFixedData[i8r], &f2 = cellsFixedData[last];
				if(f.el == f2.el || f.pl == f2.pl)
					continue;
			}
			if(!Is_OK_Suite(i8r))
				continue;
			UL_SEARCH ulsn(this);
			ulsn.el_used.Set(ele); 
			if(ulsn.Add_Chain(i8r))
				return 1;
		}
	}// end for
	return 0;
}

//=================================================
int UL_SEARCH::Is_OK_Suite(USHORT i8) {
	if(i8 == last)
		return 0;
	if(i8 == tcount[0])
		return 1;
	if(cells.On(i8))
		return 0; // false loop  
	const CELL_FIX &f = cellsFixedData[i8]; 
	if(elcpt[f.el] > 1 || elcpt[f.pl + 9] > 1 || elcpt[f.eb + 18] > 1)
		return 0;
	// for the time being, I see no more exclusion
	return 1;
}
//  entry action 0 is the start 4.6.  store after
//       one digit in excess and 

//==================================================
int UL_SEARCH::Loop_OK(int action) {
	//UL_Mess("potential loop",1);
	if(parity.f)
		return 0; // must be even number everywhere
	if(!ParityCheck()) // check for more than 0 solutions
		return 0;
	if(!action) // split processing depending on size of the loop
		if(line_count>7) {
			parentpuz->tult.Store(*this);
			return 0;
		}
		else
			action++;
	// les deux ci-dessous sortent en 4.6 et 4.7; voir l'origine de l'écart (nb de pas???)
	if(action == 1 && nadds < 2) { //one cell with adds rating 4.6 revérifié, c'est bien 4.6
		USHORT iu = adds[0];
		if(parentpuz->T81t[iu].Changey(*parentpuz, chd)) {
			UL_Mess("one cell with extra digits ", 1);
			return 1;
		}
	} // nothing if redundancy with another loop

	// now one digit in excess ++++++++++++
	if(action == 1 && (cht.bitCount() == 3)) {
		BF81 zi;
		zi.SetAll_1();
		for(int i = 0; i < nadds; i++)
			zi &= cellsFixedData[adds[i]].z;
		if(parentpuz->T81->Clear(zi, (cht - chd).First())) {
			UL_Mess(" one digit in excess", 1);
			return 1;
		}         
	}

	// action 1, launching the process common to UR/UL
	if(nadds == 2) { // type 2;3;4 must be same object 
		if(!(cellsFixedData[adds[0]].ObjCommun(&cellsFixedData[adds[1]])))
			return 0;	
		int ir = parentpuz->ur.StartECbi(adds[0], adds[1], chd, action);
		if(ir == 1)
		{UL_Mess("action UL / 2 cells)", 1);
		return 1;
		}   
	}
	// store it if action 0 
	if(action < 2) {
		parentpuz->tult.Store(*this);
		return 0;
	}

	//UL_Mess("nothing",1);
	return 0;
}
//-----  
void UL_SEARCH::UL_Mess(const char * lib, int pr) const { // et flag des "faits"
	EE->Enl();
	EE->E("UL loop nadds=");
	EE->E(nadds);
	EE->E(" count=");
	EE->E(line_count - 1);
	EE->E(" rating=");
	EE->E(parentpuz->difficulty);
	for(int i = 0; i < line_count; i++) {
		EE->Esp();
		EE->E(cellsFixedData[tcount[i]].pt);
	}
	EE->Enl();
	if(pr) {
		EE->E("active due to " );
		EE->Enl(lib);
	}
}
 
/*           <<<<<< EVENT family classes >>>>>>>>>>

  EVENT is the name given to a state of the PM where a specific pattern occurs

  eg  XWing, Nacked pait, Hidden pair ...

  Any event has 3 component

  (1) the set of candidates that must be false to have the "event"
  (2) the event description (candidates)
  (3) the set of candidates cleared if the "event" is established

  set (1) is loaded in the table of sets
  set (3) is an EVENLOT
  set (2) is located in the EVENT


  all potential events are search for each cycle and stored in TEVENT

*/

USHORT event_vi = 1024;

EVENTLOT::EVENTLOT(PUZZLE * parent,FLOG * EE,BF81 &x, USHORT ch) {
	itcd = 0;
	for(int i = 0; i < 81; i++)
		if(x.On(i))
			AddCand(parent,EE,i, ch);
}


void EVENTLOT::AddCand(PUZZLE * parentpuz,FLOG * EE,USHORT cell, USHORT ch) {
	tcd[itcd++] = parentpuz->zpln.indexc[ch*81 + cell];
}

int EVENTLOT::GenForTag(PUZZLE * parentpuz,FLOG * EE,USHORT tag, WL_TYPE type) const {
	for(int i = 0; i < itcd; i++) {
		USHORT cand2 = tcd[i], tag2 = (cand2 << 1) ^ 1;
		if(parentpuz->zcf.Isp(tag, tag2))
			continue;// only if not yet defined
		if(0 && (tag ==(tag2 ^ 1))) {
			EE->E("gen contraire");
			Image(parentpuz,EE);
			return 1;
		}

		if(tag == tag2)
			continue;// redundancy
		if(type == wl_ev_direct)
			parentpuz->zcf.LoadEventDirect(tag >> 1, cand2); 
		else
			parentpuz->zcf.LoadEventTag(tag, cand2);
	}
	return 0;
}

void EVENTLOT::Image(PUZZLE * parentpuz,FLOG * EE) const {
	EE->E(" evenlot itcd=");
	EE->E(itcd);
	for(int i = 0; i < itcd; i++) {
		EE->Esp();
		parentpuz->zpln.Image(tcd[i]);
	}
	EE->Enl();
}

             //============================


void EVENT::Load(USHORT tage, EVENT_TYPE evt, const EVENTLOT & evb, const EVENTLOT & evx) {
	tag = tage;
	type = evt;
	evl = evb;
	ntcand = evx.itcd;
	if(ntcand > 4)
		ntcand = 4;
	for(int i = 0; i < ntcand; i++)
		tcand[i] = evx.tcd[i];
}

int EVENT::IsPattern (USHORT cand) const {
	for(int i = 0; i < ntcand; i++)
		if(cand == tcand[i])
			return 1;
		return 0;
}

void EVENT::Image(PUZZLE * parentpuz,FLOG * EE) const {
	if(!parentpuz->options.ot)
		return;
	EE->E(" event image tag=");
	EE->E(tag);
	EE->E(" type=");
	EE->E(type); 
	for(int i = 0; i < ntcand; i++) {
		EE->Esp();
		parentpuz->zpln.Image(tcand[i]);
	} 
	evl.Image(parentpuz,EE);
}

void EVENT::ImageShort(PUZZLE * parentpuz,FLOG * EE) const {
	static const char * tlib[]={"pointing rc","pointing b","naked pair","hidden pair","Xwingr","Xwingc"};
	if(!parentpuz->options.ot)
		return;
	EE->E("\t event   ");
	EE->E(tlib[type]); 
	EE->E("\t\t");
	for(int i = 0; i < ntcand; i++) {
		EE->Esp();
		parentpuz->zpln.Image(tcand[i]);
	} 

}
               //================================

void TEVENT::SetParent(PUZZLE * parent,FLOG * fl) {
	parentpuz = parent; 
	EE = fl;
}

/* we have found the 2 sets (EVENLOT) linked to the event
   create the entry in the table TEVENT
   if first set is more than one create entry in TSET
   else generate directly conflicts for candidates in conflict
        if they don't belong to the pattern 
   evt event type
   eva lot of candidates creating the event
   evb lot of candidates not valid if the pattern is established
   evx candidates of the pattern
   */


// initial seacrh with the priority to the first found
void TEVENT::LoadAll() {
	Init();
	LoadLock();
	LoadPairs();
	LoadXW();
}


void TEVENT::EventBuild(EVENT_TYPE evt, EVENTLOT & eva, EVENTLOT & evb, EVENTLOT & evx) {
	if(eva.itcd < 1 || evb.itcd < 1)
		return;
	// if only one in the set, go to gen and return
	if(eva.itcd == 1) {
		USHORT cw = eva.tcd[0]; 
		if(0 && parentpuz->options.ot) {
			EE->E("gen direct event type=");
			EE->E(evt);
			EE->Esp(); 
			eva.Image(parentpuz,EE);
			evb.Image(parentpuz,EE);
			EE->Enl();
		}
		evb.GenForTag(parentpuz, EE,(cw << 1) ^ 1, wl_ev_direct);
		return;
	}
	if(eva.itcd > chx_max)
		return; // forget if too big	 
	if(it >= event_size) {
		parentpuz->Elimite("TEVENT limite atteinte");
		return;
	}
	USHORT evv = event_vi + (it);
	t[it++].Load(evv, evt, evb, evx); 
	eva.tcd[eva.itcd] = evv;
	parentpuz->zcx.ChargeSet(eva.tcd, eva.itcd + 1, SET_set); // set loaded
}

 /* the event has been created by  "tagfrom" for the event "tagevent"
    find the corresponding element in the talbe and generate the weak links
	*/
int TEVENT::EventSeenFor(USHORT tagfrom, USHORT tagevent) const {
	USHORT candfrom = tagfrom >> 1, ind = tagevent - event_vi;  
	if(ind < 1 || ind >= it)
		return 0; // should never be
	if(t[ind].IsPattern(candfrom))
		return 0;// ?? do nothing if part of the pattern not 100% sure
	if(t[ind].evl.GenForTag(parentpuz,EE,tagfrom, wl_event)) { // diag a supprimer
		t[ind].Image(parentpuz,EE);
		return 1;
	} 
	return 0; 
}

/* process to locate all potential XW
   load the corresponding tags and sets
   
*/
void TEVENT::LoadXW() {
	for(int i1 = 0; i1 < 8; i1++) {
		for(int j1 = 0; j1 < 8; j1++) {  // first cell
			USHORT cell1 = I81::Pos(i1, j1);
			if(parentpuz->T81t[cell1].v.ncand < 2)
				continue;
			for(int i2 = i1 + 1; i2 < 9; i2++) {
				USHORT cell2 = I81::Pos(i2, j1);
				if(parentpuz->T81t[cell2].v.ncand < 2)
					continue;
				BF16 ch2 = parentpuz->T81t[cell1].v.cand & parentpuz->T81t[cell1].v.cand;
				if(!ch2.f)
					continue; // must have at least a common digit
				for(int j2 = j1 + 1; j2 < 9; j2++) {
					USHORT cell3 = I81::Pos(i1, j2);
					if(parentpuz->T81t[cell3].v.ncand < 2)
						continue;
					USHORT cell4 = I81::Pos(i2, j2);
					if(parentpuz->T81t[cell4].v.ncand < 2)
						continue;
					BF16 cht = ch2 & parentpuz->T81t[cell3].v.cand & parentpuz->T81t[cell4].v.cand;
					if(!cht.f)
						continue; // must have at least a common digit
					// we now explore all possible row and column XWings in that group
					for(int ich = 0; ich < 9; ich++) {
						if(cht.On(ich)) { // all possible digits
							// build the row  and the column sets
							EVENTLOT evr, 
								     evc, 
									 evx;
							LoadXWD(ich, i1, i2, j1, j2, evr, evx);
							if(!evr.itcd)
								continue; // pure XWing
							LoadXWD(ich, j1 + 9, j2 + 9, i1, i2, evc, evx); // now can not be a pure XWing
							EventBuild(evxwcol, evr, evc, evx);
							EventBuild(evxwrow, evc, evr, evx);
						}
					} //ich
				} // j2
			} // i2
		} // j1
	}// i1
}

/* we have identified an XW pattern
   generate if any the set or the direct event weak links  */
void TEVENT::LoadXWD(USHORT ch, USHORT el1, USHORT el2, USHORT p1, USHORT p2, EVENTLOT & eva, EVENTLOT & evx) {
	REGION_CELL el1d = parentpuz->alt_index.tchbit.el[el1].eld[ch], el2d = parentpuz->alt_index.tchbit.el[el2].eld[ch];
	for(int i = 0; i < 9; i++)
		if(el1d.b.On(i))
			if((i - p1) && (i - p2))
				eva.AddCand(parentpuz, EE, cellsInGroup[el1][i], ch);
			else
				evx.AddCand(parentpuz, EE, cellsInGroup[el1][i], ch);
	for(int i = 0; i < 9; i++)
		if(el2d.b.On(i))
			if((i - p1) && (i - p2))
				eva.AddCand(parentpuz, EE, cellsInGroup[el2][i], ch);
			else
				evx.AddCand(parentpuz, EE, cellsInGroup[el2][i], ch);
}

void TEVENT::LoadPairs() { // all rows, columns, and boxes  
	for(int iel = 0; iel < 27; iel++) {
		// pick up 2 unknown cells in that region
		for(int i1 = 0; i1 < 8; i1++) {
			USHORT cell1 = cellsInGroup[iel][i1];
			if(parentpuz->T81t[cell1].v.ncand < 2)
				continue;
			for(int i2 = i1 + 1; i2 < 9; i2++) {
				USHORT cell2 = cellsInGroup[iel][i2];
				if(parentpuz->T81t[cell2].v.ncand < 2)
					continue;
				LoadPairsD(cell1, cell2, iel);
			}
		} // end i1;i2
	}// end iel
}
// work on a pair 
void TEVENT::LoadPairsD(USHORT cell1, USHORT cell2, USHORT iel) {
	BF16 ch2 = parentpuz->T81t[cell1].v.cand & parentpuz->T81t[cell2].v.cand,
		chor = parentpuz->T81t[cell1].v.cand | parentpuz->T81t[cell2].v.cand; // pour traiter le cas 1 commun
	if(ch2.bitCount() < 2)
		return; // non il faudrait aussi accepter 1 commun à revoir
	// nothing to do if box/row and box/col (already done)
	const CELL_FIX &p1 = cellsFixedData[cell1], &p2=cellsFixedData[cell2];
	if(iel > 17 && ((p1.el == p2.el) || (p1.pl == p2.pl)))
		return;
	for(int i1 = 0; i1 < 8; i1++) {
		if(!ch2.On(i1)) 
			continue;
		for(int i2 = i1 + 1; i2 < 9; i2++) {
			if(!ch2.On(i2))
				continue;
			//all pairs must be processed
			// buid the set for nacked pair	 
			EVENTLOT nack,  hid1, 
				     hid2,  evx; 
			BF16 com(i1, i2), v1 = parentpuz->T81t[cell1].v.cand, v2 = parentpuz->T81t[cell2].v.cand;
			for(int j = 0; j < 9; j++) {
				if(v1.On(j))
					if(com.On(j))
						evx.AddCand(parentpuz,EE,cell1, j);
					else
						nack.AddCand(parentpuz,EE,cell1, j);
				if(v2.On(j))
					if(com.On(j))
						evx.AddCand(parentpuz,EE,cell2, j);
					else
						nack.AddCand(parentpuz,EE,cell2, j);
			}
			// build the set for hidden pair in el and generate the event
			PairHidSet(cell1, cell2, iel, com, hid1);

			if(hid1.itcd <= chx_max)
				EventBuild(evpairhidden, hid1, nack, evx);
			    // unless same box, finish with the naked pai
			if(iel > 17 || (p1.eb - p2.eb)){
				if(nack.itcd <= chx_max)
					EventBuild(evpairnacked, nack, hid1, evx);
				continue;
			}
				// i same box, do hidden in the box and naked for both

			PairHidSet(cell1, cell2, p2.eb + 18, com, hid2);
			if(hid2.itcd <= chx_max)
				EventBuild(evpairhidden, hid2, nack, evx);
			if(nack.itcd <= chx_max){
				  // do it for both hid1 and hid2
				for(int ia=0;ia<hid2.itcd;ia++)
					if(hid1.itcd<30)
						hid1.tcd[hid1.itcd++]=hid2.tcd[ia];
				EventBuild(evpairnacked, nack, hid1, evx);
			}

		}
	}
}


void TEVENT::PairHidSet(USHORT cell1, USHORT cell2, USHORT el, BF16 com, EVENTLOT & hid) {
	for(int i = 0; i < 9; i++) {
		int cell = cellsInGroup[el][i];
		if((cell == cell1) || (cell == cell2))
			continue;
		CELL_VAR p = parentpuz->T81t[cell].v;
		if(p.ncand < 2)
			continue;
		BF16 w = p.cand&com;
		if(!w.f)
			continue;
		for(int j = 0; j < 9; j++)
			if(w.On(j))
				hid.AddCand(parentpuz,EE,cell, j);
	}
}

/* prepare all claiming pointing sets
   to be done in each row/box row/col if there is a digit not locked
*/

void TEVENT::LoadLock() {
	for(int iel = 0; iel < 18; iel++) {
		for(int ib = 18; ib < 27; ib++) {
			for(int ich = 0; ich < 9; ich++) {
				BF81 chel = parentpuz->c[ich] & cellsInHouseBM[iel];  // the elem pattern for the ch
				if(chel.IsEmpty())
					continue; // nothing  
				BF81 chbcom = chel & cellsInHouseBM[ib]; // common part with the block
				if(chel == chbcom)
					continue; //  already locked
				if(chbcom.Count() < 2)
					continue; // nothing to do I guess
				chel -= chbcom; // set in row col
				BF81 chb = (parentpuz->c[ich] & cellsInHouseBM[ib]) - chbcom; // set in box

				// check what does SE if evrc,evb,evx all one candidate ?? 

				EVENTLOT evrc(parentpuz,EE, chel, ich), 
					     evb(parentpuz,EE,chb, ich), 
						 evx(parentpuz,EE,chbcom, ich);
				EventBuild(evlockrc, evrc, evb, evx);
				EventBuild(evlockbox, evb, evrc, evx);
			}
		}
	}
}

/* code for preliminary tests
 status of table after having loaded events
 */
void TEVENT::LoadFin() {
	if(!parentpuz->options.ot)
		return;

	EE->E("check after having loaded events nb events=");
	EE->Enl(it);
	EE->E("zcx izc=");
	EE->Enl(parentpuz->zcx.izc);
	for(int i = 1; i < it; i++) {
		EVENT ev = t[i]; 
		EE->E("ev type=");
		EE->E(ev.type);
		EE->E(" tag=");
		EE->E(ev.tag);
		for(int j = 0; j < ev.ntcand; j++) {
			EE->Esp();
			parentpuz->zpln.Image(ev.tcand[j]);
		}
		ev.evl.Image(parentpuz,EE);
	}
	parentpuz->zcx.Image();
}

/*                  <<<<< SEARCH_UR >>>>>>>>>>>>>>>>

this is a class used to searhc ( and store) URs

*/


void SEARCH_UR::SetParent(PUZZLE * parent , FLOG * xx,
	               CELL * tae, CELL * tre,
				   REGION_INDEX * tchele ) {	
    parentpuz=parent;
	EE=xx;
	ta = tae;
	tr = tre;
	tchel = tchele;
}

int SEARCH_UR::GetElPlus() {
	return CELLS_FIX::GetLigCol(pp1,pp2);
} // assuming nplus=2

int SEARCH_UR::IsDiag() {
	if(DIVF::IsObjet(deux[0],deux[1]))
		diag=0;
	else
		diag = 1;
	return diag;
}

int SEARCH_UR::Jumeau(USHORT a,USHORT b,USHORT ch) {
	USHORT el = CELLS_FIX::GetLigCol(a,b);
	return Jum(el, ch);
}

void SEARCH_UR::ImageRI(const char * lib,USHORT a) {
	EE->E(lib);
	EE->E(tr[a].f->pt);
	EE->E(" ");
	EE->E(tr[a].scand);
}

void SEARCH_UR::ImageRI(const char * lib) {
	if(!parentpuz->options.ot) return;
	EE->E( "->UR" );
	EE->E(lib);
	ImageRI(" P1=", ia);
	ImageRI(" P2=", ib);
	ImageRI(" P3=", ic);
	ImageRI(" P4=", id);
	EE->Enl();
}

//former _05a_rin_el.cpp follows

/* full processing for 2 cells with adds in one object UR or UL
   can be bivalue, "hidden locket set" or "locked set"
   always the lowest rating found
   the rule has been copied from SE code analysis adjusted to lksudokuffixed8 veersion.
   here "basis" is the rating entry for the specific object
       UR basi=4.5 
       UL up to 6 cells basis=4.6 (+0.1)  7_8 cells 4.7 (+0.2)  10_.. 4.8 (+0.3)
   basis        one digit bivalue
   basis +0.1 (n-1) hidden/nacked sets of "n" cells

   for hidden and naked sets, the lowest rating is taken depending on "n" values 

summary of rating having "equalled" hidden and naked as in lksudoku 1.2.5.0

 URUL 
 cells -->  4    6    8   >=10

 pair     4.6  4.7  4.8  4.9    action 2
 triplet  4.7  4.8  4.9  5.0    action 3
 quad     4.8  4.9  5.0  5.1    action 4


*/
int SEARCH_UR::T2(USHORT action) {
	int ir1 = 0, ir2 = 0, iel = GetElPlus();
	if(iel >= 0) {
		ir1 = T2_el(iel, action);
		if(ir1 == 1)
			return 1;
	}
	int eb = cellsFixedData[pp1].eb;
	if(eb == cellsFixedData[pp2].eb)
		ir2 = T2_el(eb + 18, action);
	if(ir2 == 1)
		return 1;
	if((ir1 + ir2) == 0)
		return 0;
	else
		return 2;
}

// same but el is now identified 
int SEARCH_UR::T2_el(USHORT el, USHORT action) {
	//look mode 1 for a bivalue of one of the common digits
	if(0) {
		ImageRI("");
		EE->E("UR/UL el ");
		EE->E(el + 1);
		EE->E(" action=");
		EE->E(action);
		EE->E(" pp1=");
		EE->E(cellsFixedData[pp1].pt);
		EE->E(" pp2=");
		EE->Enl(cellsFixedData[pp2].pt);
	}
	if(action == 1) { //   this is a "basis"  
		if(Jum(el, chc1)) {
			int ir1 = parentpuz->T81t[pp1].Changex(*parentpuz, chc2) + parentpuz->T81t[pp2].Changex(*parentpuz, chc2);
			if(ir1) {
				EE->E("UR/UL bivalue ");
				EE->Enl(chc1+1);
				return 1;
			}
		}
		else if(Jum(el,chc2)) {
			int ir1 = parentpuz->T81t[pp1].Changex(*parentpuz, chc1) + parentpuz->T81t[pp2].Changex(*parentpuz, chc1);
			if(ir1) {
				EE->E("UR/UL bivalue ");
				EE->Enl(chc2 + 1);
				return 1;
			}
		}
	}

	// first look  for cells having a potential for hidden/direct locked sets
	int ir = 0;
	aig_hid = 0;
	nth = ntd = nnh = 0;
	wh = wc;
	wd = wr;
	wnh = wr;
	wnd = wc;
	wdp.f = 0;
	zwel.SetAll_0(); // to prepare clearing of naked sets
	for(int i = 0; i < 9; i++) {
		int ppi = cellsInGroup[el][i];
		if((ppi == pp1) || (ppi == pp2))
			continue;
		CELL_VAR v = parentpuz->T81t[ppi].v;
		if(v.typ)
			continue; // given or assigned 
		// count and store cells including common digits 
		zwel.Set(ppi);//will contain all active cells out of the UR at the end
		BF16 ww = v.cand & wc;
		if(ww.f) {
			th[nth++] = ppi;
			wh |= v.cand;
			if(ww.f == wc.f)
				aig_hid = 1;
		} 
		else { // naked locked set  can not have common digits	  
			wnh |= v.cand; 
			tnh[nnh++] = ppi; // nnh to check at least one cell without common digits
			ww = v.cand & wr; 
			if(ww.f) {
				td[ntd++] = ppi;
				wd |= v.cand;
			}
			else
				wnd |= v.cand;
		}
	}
	//ImageRI(" ");EE->E(" nth=");EE->E(nth );EE->E(" wh=");EE->Enl(wh.String() );

	if(!nnh)
		return 0;//all cells have common digits nothing to do

	//EE->E(" nth=");EE->E(nth );EE->E(" nnh=");EE->E(nnh );EE->E(" nd=");EE->Enl(ntd );

	if(action < 2)
		return 2; // store it if not basic
	//  hidden pair 
	if(nth < 2 && (action == 2)) {  //  hidden pair if active
		if(parentpuz->T81t[th[0]].Keepy(*parentpuz, wc)) {
			EE->Enl("UR/UL hidden pair");
			return 1;
		}
	}

	// we now look  for  a hidden or nacked set of the appropriate length

	if(T2_el_set_hidden(action - 1))
		return 1;
	if(T2_el_set_nacked(action - 1))
		return 1;
	return 0;
}

//former _05a_rin_el2.cpp follows

// we are now looking for nacked or hidden sets not  pair
// we have not so many possible patterns, 
// we try to catch them per pattern
// it need more code, but it should be faster and easier to debug

// look for hidden  
int SEARCH_UR::T2_el_set_hidden(USHORT len) {
	char ws[10];
	// first pattern, take it if direct 
	if(nth==len)   // not the expected length
	{ // identify extra digits in hidden locked set
		BF16 whh=wh-wnh;  // all digits of the assumed locked set
		if(whh.bitCount() - (nth + 1)) return 0;
		//go for the a hidden set if active
		int ir1=0;
		for(int i=0;i<nth;i++)  ir1+=parentpuz->T81t[th[i]].Keepy(*parentpuz, whh); 
		if(ir1) { EE->E("UR/UL hls whh="); EE->Enl(whh.String(ws));
		EE->Enl("UR/UL hidden locked set"); return 1;}	
	}
	if(nth==2 && len==3) 
		// second pattern with nth=2 but a hidden quad 
		// adding another cell
	{
		for(int i=0;i<nnh;i++) {
			BF16 wa=wh|parentpuz->T81t[tnh[i]].v.cand,wb,wx;
			for(int j=0;j<nnh;j++) if(j-i) wb|=parentpuz->T81t[tnh[j]].v.cand;
			wx=wa-wb-wr; // must not be an extra digit included in the UR
			if(wx.bitCount()==4) // we got it
			{int ir1=0; ir1+=parentpuz->T81t[tnh[i]].Keepy(*parentpuz, wx);
			for(int k=0;k<nth;k++)  ir1+=parentpuz->T81t[th[k]].Keepy(*parentpuz, wx); 
			if(ir1) { EE->E("UR/UL hls wx="); EE->Enl(wx.String(ws));
			EE->Enl("UR/UL hidden locked set"); return 1;}	
			}
		}
	}

	return 0;
}

// look for nacked sets n
int SEARCH_UR::T2_el_set_nacked(USHORT len)
 {if(ntd<(nautres-1)) return 0;  // minimum without extra digit

 if(ntd && (nautres==2)&&(len==1)) // look for a nacked pair
  {for(int i=0;i<ntd;i++) 
   {int i8=td[i];BF16 ww=parentpuz->T81t[i8].v.cand;  
    if( ww.f==wr.f) // we got it     
	{ BF81 zwel1=zwel; zwel1.Clear(i8);
	  if(parentpuz->T81->Clear(zwel1,wr  ))
		{ EE->Enl("UR/UL nacked pair");return 1;}
	}}
  }

 if(len<2) return 0;// nothing else if a pair is expected
if(0) { EE->E("look for nacked len ="); EE->E( len);
          EE->E(" nnh ="); EE->Enl( nnh);}
// first   pattern is nacked triplet no extra digit
if(nautres==3 && ntd>2 && len==2)
 { BF81 zwel1=zwel; int n=0;  
   for (int i=0;i<ntd;i++)      if(!(parentpuz->T81t[td[i]].v.cand-wr).f)  
	                           { n++; zwel1.Clear(td[i]);}
   if(n==2 && parentpuz->T81->Clear(zwel1,wr  ))
		       { EE->Enl("UR/UL nacked LS3");return 1;}
 }

// second  pattern : directly correct (may be with extra digit)
if(((ntd + 1) == wd.bitCount()) && (len == ntd)) {
	BF81 zwel1=zwel; 
	for (int i=0;i<ntd;i++)  zwel1.Clear(td[i]);
	if(parentpuz->T81->Clear(zwel1,wd  ))
	{ EE->E("UR/UL nacked LS");EE->Enl(ntd+1);return 1;}
}
// third  pattern is  "one cell in excess"
//should cover several situations 
if(ntd>=nautres && (len==(ntd-1)))
 { for (int i=0;i<ntd;i++)
	{BF16 wdx=wr;	 // wr is the minimum fo the nacked lock set
	 for (int j=0;j<ntd;j++) if(j-i) wdx|=parentpuz->T81t[td[j]].v.cand;
	 // check if now correct
	      if(ntd == wdx.bitCount()) //  one cell less ntd must be number of digits
       { BF81 zwel1=zwel; 
        for (int j=0;j<ntd;j++)  if(j-i)zwel1.Clear(td[j]);
		if(parentpuz->T81->Clear(zwel1,wdx  ))
		   { EE->E("UR/UL nacked LS");EE->Enl(ntd);return 1;}
       }
     }
  }

// Fourth  pattern is  "2 cells in excess"
//could cover several situations 
if(ntd>=(nautres+1) && (len==(ntd-2)))
 { for (int i=0;i<(ntd-1);i++)for (int k=i+1;k<ntd;k++)
	{BF16 wdx=wr;	 // wr is the minimum fo the nacked lock set
	 for (int j=0;j<ntd;j++) if((j-i) && (j-k))
		    wdx|=parentpuz->T81t[td[j]].v.cand;
	 // check if now correct
	if(ntd == (wdx.bitCount() + 1))  
       { BF81 zwel1=zwel; 
        for (int j=0;j<ntd;j++)  if((j-i) && (j-k))
			 zwel1.Clear(td[j]);
		if(parentpuz->T81->Clear(zwel1,wdx  ))
		   { EE->E("UR/UL nacked LS");EE->Enl(ntd-1);return 1;}
       }
     }
  }
// fifth pattern,  we look now for something with all extra cells "over all"
// have an example of naked quad, but can be simpler
if(nnh>=nautres && (len==nnh))
 {BF16 wdx; int ir=0;
  for (int i=0;i<nnh;i++) wdx|=parentpuz->T81t[tnh[i]].v.cand;
  if(nnh == (wdx.bitCount() - 1) && ((wdx&wr).f == wr.f) ) 
    { for (int i=0;i<nth;i++)  ir+=parentpuz->T81t[th[i]].Changey(*parentpuz, wdx);
      if(ir)  { EE->E("UR/UL nacked LS");EE->Enl(wdx.bitCount());return 1;}
    } 
 }

// fifth all extra cells except one
if(nnh>=nautres && (len==(nnh-1)))
 {for (int j=0;j<nnh;j++)
  {BF16 wdx; int ir=0;
	  for (int i=0;i<nnh;i++) if(j-i)  wdx|=parentpuz->T81t[tnh[i]].v.cand;
  if((wdx&wr).f-wr.f) continue;
  if(nnh == wdx.bitCount()) 
    { BF81 zwel1=zwel; 
      for (int i=0;i<nth;i++)  ir+=parentpuz->T81t[th[i]].Changey(*parentpuz, wdx);
	  ir+=parentpuz->T81t[tnh[j]].Changey(*parentpuz, wdx);
      if(ir)  { EE->E("UR/UL nacked LS");EE->Enl(wdx.bitCount());return 1;}
    } 
  }
 }
// sixth all extra cells except 2
if(nnh>=(nautres+1) && (len==(nnh-2)))
 { EE->E("look for all extra -2 nnh=");EE->Enl(nnh);
 for (int j=0;j<nnh-1;j++)for (int k=j+1;k<nnh;k++)
  {BF16 wdx; int ir=0;
	  for (int i=0;i<nnh;i++) if((j-i)  && (k-i))
		     wdx|=parentpuz->T81t[tnh[i]].v.cand;
   if((wdx&wr).f-wr.f) continue;
   if(nnh == wdx.bitCount() + 1) 
    { BF81 zwel1=zwel; 
      for (int i=0;i<nth;i++)  ir+=parentpuz->T81t[th[i]].Changey(*parentpuz, wdx);
	  ir+=parentpuz->T81t[tnh[j]].Changey(*parentpuz, wdx); ir+=parentpuz->T81t[tnh[k]].Changey(*parentpuz, wdx);
      if(ir)  { EE->E("UR/UL nacked LS");EE->Enl(wdx.bitCount());return 1;}
    } 
  }
 }


return 0;}

/*

// sixth pattern,  we look now for something with 3 extra cells "over all"
// has been seen 
if(nnh>=(nautres+1)&& (len==(nnh-3)) )
 {EE->Enl("look for nacked LS + extra digit less 3 cells");
	 for (int i1=0;i1<nnh-2;i1++)for (int i2=i1+1;i2<nnh-1;i2++)
     for (int i3=i2+1;i3<nnh;i3++)
	{BF16 wdx=wr;	 // wr is the minimum fo the nacked lock set
	 for (int j=0;j<nnh;j++) 
		  if((j-i1) &&(j-i2)&&(j-i3))    wdx|=parentpuz->T81t[tnh[j]].v.cand;
	 // check if now correct
     if((nnh-2)==wdx.QC()) 
       { BF81 zwel1=zwel; 
        for (int j=0;j<nnh;j++)  
			 if((j-i1) &&(j-i2)&&(j-i3))zwel1.Clear(tnh[j]);
         if(parentpuz->T81->Clear(zwel1,wdx  ))
		   { EE->E("UR/UL nacked LS");EE->Enl(nnh-2);return 1;}
       }
     }
  }	 
*/

//a piece of former _05b_RI_2N.cpp follows

// one posible location for a UR;
// no assigned position, 2 and only 2 common digits

int SEARCH_UR::RIDx(int i1,int i2,int c1,int c2) {
	ia = I81::Pos(i1, c1);
	ib = I81::Pos(i1, c2);
	ic = I81::Pos(i2, c1);
	id = I81::Pos(i2, c2);
	char * gr = parentpuz->gg.pg;
	if((gr[ia] - '0') || (gr[ib] - '0') || (gr[ic] - '0') || (gr[id] - '0'))
		return 0;

	if(Setw() - 2)
		return 0;
	CalcDeux();   
	if(ndeux == 3) {
		ta[pp1].Changey(*parentpuz, wc);
		ImageRI("1");
		return 1;
	}// type 1
	Setwou();
	GenCh(); 
	if((ndeux - 2) || IsDiag())
		if(ndeux == 1)
			return RID3();
		else
			return 0; // not processed

	// if one digit active, do it now 4.5
	if(nautres == 1) { // un chiffre en lig/col  ou diagonal
		if(parentpuz->Keep(ch1, pp1, pp2)) {
			ImageRI(" one digit active");
			return 1;
		}
	}
	// if one of the common digit is a bivalue, do it as well it is 4.5
	while((tr[ia].v.ncand > 2) || (tr[ic].v.ncand > 2)) {
		USHORT pw = ia;
		ia = ib;
		ib = id;
		id = ic;
		ic = pw;
	}// sort cells 
	int ir = T2(1); // we  want only rating 4.5 as "good" so far
	if(ir == 1) {
		ImageRI(" action from object");
		return 1;
	} // it was ok
	if(ir)
		parentpuz->urt.Store(this); 
	return 0; //store if something else seen
}

int SEARCH_UR::RID3() {

	if(nautres - 1)
		return 0;
	BF81 zw;
	zw.SetAll_1();

	if(tr[ia].v.ncand == 3)
		zw &= cellsFixedData[ia].z;
	if(tr[ib].v.ncand == 3)
		zw &= cellsFixedData[ib].z;
	if(tr[ic].v.ncand == 3)
		zw &= cellsFixedData[ic].z;
	if(tr[id].v.ncand == 3)
		zw &= cellsFixedData[id].z;

	zw &= parentpuz->c[ch1];

	if(zw.IsNotEmpty()) {
		ImageRI(" UR one digit active");	
		return parentpuz->T81->Clear(zw,ch1);
	}
	return 0;
}

/*                 <<<<  CANDIDATE    >>>>>
  
   that class contains in sequence all candidates of the PM
   in the process, each candidate can be identfied as its index in that table

   entry 0 is free kept for invalid return

   a BFCAND can show any binary property of the candidates
   a BFTAG (2* BFCAND size) shows the properties of the status TRUE or FALSE of a candidate

   The table is updated at each cycle
*/


void CANDIDATE::Clear(PUZZLE &puz, CELL * tw){
	tw[ig].Changex(puz, ch);
}

void CANDIDATE::Image(FLOG * EE,int no) const {
	EE->Esp();
	if(no)
		EE->E("~");
	EE->E(ch+1);
	EE->E(cellsFixedData[ig].pt); 
}

void CANDIDATES::SetParent(PUZZLE * parent,FLOG * fl){
	parentpuz = parent;
	EE = fl;
}
	
void CANDIDATES::Clear(USHORT c) 	{
		zp[c].Clear(*parentpuz, parentpuz->T81t);
	}


/* creates the table of candidates
   and the direct index cell+ digit -> cand
   looks for true candidates
   init table zcf
   set up the usefull limit for BFCAND and CB1024 
 */


void CANDIDATES::Init() {
	ip=1;
	candtrue.SetAll_0();
	for(int i = 0; i < 9 * 81; i++)
		indexc[i]=0;
	for(UCHAR i = 0; i < 81; i++) {
		if(parentpuz->T81t[i].v.ncand < 2)
			continue;
		BF16 chs = parentpuz->T81t[i].v.cand;  
		for(UCHAR j = 0; j < 9; j++)
			if(chs.On(j)) {
				zp[0].Charge(i, j); 
				if(parentpuz->solution[i] == (j + '1'))
					candtrue.Set(ip);
				indexc[81 * j + i] = Charge0();
			}
	}
	parentpuz->col = 2 * ip;
}


/* just put zp[0] in the next position 
   check for free room just in case
   if no free room, puz.stop_rating is set to 1 thru Elimite
*/
USHORT CANDIDATES::Charge0(){
	if(ip>=zpln_lim){  
		parentpuz->Elimite("TZPLN"); 
		return 0;
	}
	USHORT ir=ip; 
	zp[ip++]=zp[0]; 
	 return ir;
}



/* send in INFERENCES all weak links
   if it is a bi value, then 2 weak links are created
      a - b  and   ~a - ~b
   one entry for 'X' mode and one entry for 'Y' mode
   in 'X' mode we check for no double entry

*/

// this is a process specific to 'Y' mode
// only cell bivalues + 

void  CANDIDATES::CellStrongLinks(){ 
	for(int i=0;i<81;i++) 
     if(parentpuz->T81t[i].v.ncand==2) {
		iptsch=0; 
		for(int ich=0,jp=i;ich<9;ich++,jp+=81) {
			int j=indexc[jp];
			if(j)  
				ptsch[iptsch++]=j;  
		}
	  parentpuz->zcf.LoadBivalue(ptsch[0],ptsch[1]);
     }
}
/* generate weak and strong links from cells.
   if biv=1, no generation of strong links
   (if not, biv=3)
*/


void  CANDIDATES::CellLinks() 
{ el=0; // to shunt the filter in WeaklinksD
 for(int i=0;i<81;i++) 
	 if(parentpuz->T81t[i].v.ncand<2)
		 continue;
    else {
			iptsch=0; 
			for(int ich=0,jp=i;ich<9;ich++,jp+=81){
				int j=indexc[jp];
				if(j) 
					ptsch[iptsch++]=j;   
			}
			if(iptsch==2) { 
				parentpuz->zcf.LoadBivalue(ptsch[0],ptsch[1]);      
				parentpuz->zcf.LoadBase(ptsch[0],ptsch[1]);
			}
			else  
				if(iptsch)
					WeakLinksD();
		 }
}


void  CANDIDATES::RegionLinks(USHORT ich,int biv){
	for (el=0;el<27;el++) { 
		iptsch=0;  
		if(parentpuz->alt_index.tchbit.el[el].eld[ich].n <2 )
			continue;
		for(int i=0;i<9;i++)  {
			//USHORT w=indexc[divf.el81[el][i]+81*ich];
			USHORT w=indexc[cellsInGroup[el][i]+81*ich];
			if(w) 
				ptsch[iptsch++]=w;}
			if(iptsch==2 && biv) {
				parentpuz->zcf.LoadBivalue(ptsch[0],ptsch[1]);
	            parentpuz->zcf.LoadBase(ptsch[0],ptsch[1]);
			}
		else    
			if(iptsch)
				WeakLinksD();
   }
}


void  CANDIDATES::WeakLinksD(){
	for(int j=0;j<iptsch-1;j++){// look for all pairs
		USHORT p1=ptsch[j],ig1=zp[p1].ig;
		for(int k=j+1;k<iptsch;k++){ 
			USHORT p2=ptsch[k],
				  ig2=zp[p2].ig;
	        // don't generate twice if box/row or box/column 
			if(el>17 &&   // it is a box
				(cellsFixedData[ig1].el==cellsFixedData[ig2].el  ||  cellsFixedData[ig1].pl==cellsFixedData[ig2].pl ) )
				continue;
			parentpuz->zcf.LoadBase(p1,p2)    ;  
		}  
	}
}

//---------- gen sets of candidates in a row column box 
/* changed the order of generation for 2 reasons
   closer to serate mode
   better chance to have same rating for morphs

   first generate box sets
   and do it in increasing order of the set size

*/

void CANDIDATES::GenRegionSets()  {   
	// can only be one per row col box, only if more than 2 candidates
	USHORT mch[10];    
    //  box row column increasing sise of sets
	for( int ncand=3;ncand<10;ncand++)
		for (int elx=0,el=18;elx<27;elx++,el++){
			if(el==27) el=0;
			for( int ich=0;ich<9;ich++) {
				USHORT nmch = parentpuz->alt_index.tchbit.el[el].eld[ich].n,
					  ipts = 0;
				if(nmch-ncand) 
					continue;  
				//BF81 zel=divf.elz81[el]&puz.c[ich];
				BF81 zel = parentpuz->c[ich] & cellsInHouseBM[el];
				for(int j = 1; j < ip; j++) {
					if(zp[j].ch - ich )
						continue;
					if(zel.On(zp[j].ig)) 
						mch[ipts++] = j; 
				}
				parentpuz->zcx.ChargeSet(mch, nmch, SET_base);
			}
		}

}


void CANDIDATES::GenCellsSets(){
	for(USHORT i=0;i<81;i++)  {
		USHORT n= parentpuz->T81t[i].v.ncand; 
		if(n<3 || n>chx_max) 
			continue;    
		BF16 ch8=parentpuz->T81t[i].v.cand;  
		USHORT nm=0,tm[9];    
		for(int j=0,jp=i;j<9;j++,jp+=81) 
			if(ch8.On(j)){
				tm[nm++]=indexc[jp];
			}
			parentpuz->zcx.ChargeSet(tm,nm,SET_base);    
	}  
}



void CANDIDATES::PrintImply(const USHORT * tp,USHORT np) const {
	if(!parentpuz->options.ot)
		return;
	EE->Echem();
	for(int i = 0; i < np; i++) {
		//const USHORT px = tp[i];
		ImageTag(tp[i]);
		if(i < np - 1)
			EE->E(" -> ");
	} 
	EE->Enl();
}

void CANDIDATES::PrintListe(USHORT * tp,USHORT np,int modetag) const{
	if(!parentpuz->options.ot) 
		return; 
	EE->E("candidats");
	for(int i=0;i<np;i++)  {
		USHORT px=tp[i];   
		if(modetag) 
			ImageTag(px); 
		else  
			zp[px].Image(EE);
		EE->Esp();  
	} 
 EE->Enl();
}



/*        <<<<< INFERENCES     >>>>>>>>>>>>>>

that class is the support for all weak and strong links settelement

*/
void INFERENCES::SetParent(PUZZLE * parent,FLOG * fl){
parentpuz=parent;
EE=fl;
}

int INFERENCES::DeriveCycle(int nd, int nf, int ns, int npas) {
	load_done=0;  // to check if something new comes
	hstart = h; // copy to have the situation at the start
	parentpuz->zcx.Derive(nd, nf, ns);
	if(!load_done) return 0;
	if(!npas)
		h.d.ExpandAll(*parentpuz, h.dp);
	else
		h.d.ExpandShort(*parentpuz, h.dp, npas);
	return 1;
}



// entry for basic weak link a - b
void INFERENCES::LoadBase(USHORT cd1 ,USHORT cd2) {
	Entrep(cd1 << 1, cd2 << 1);
}

// entry for bi value  a = b thru a - b and ã - ~b   
void INFERENCES::LoadBivalue(USHORT cd1, USHORT cd2) {
	load_done=1;
	Entrep((cd1 << 1) ^ 1, (cd2 << 1) ^ 1);  
	isbival.Set(cd1);
	isbival.Set(cd2);
}

// entry for weak link derived from a set  a -> b
void INFERENCES::LoadDerivedTag(USHORT tg1, USHORT cd2) {
	load_done=1;
	Plusp(tg1, cd2 << 1);  //  a -> b
}

// entry for event  a - b
void INFERENCES::LoadEventTag(USHORT tg1, USHORT cd2) {
	load_done=1;
	Plusp(tg1, (cd2 << 1) ^ 1); // a -> ~b
}

// entry for direct event  ~a - b
void INFERENCES::LoadEventDirect(USHORT cd1, USHORT cd2) {
	load_done=1;
	Plusp((cd1 <<1 ) ^ 1, (cd2 << 1) ^ 1); 
}

 /* In that process, we do in the same pass the search for
    x cycle;chain  or  y cycle;chain  or XY cycle;chain

	if nothing comes from  ?chain, there is no need to search the loop
	in case we have results from ?chain,
	we expand all and collect 
	  all eliminations ("true status"), 
	  all tags in loop
    
	for each elimination, we first look for the chain, then for the loop if any
    tchain takes care of the smaller ratings


  Note we look only in that process,  elimination found thru a "true"  state of a candidate
  nota : to get a "false" elimination we need
  ~a - ~b = b - ..... - x = ~x - ~a (only possible weak link ~x - ~y)
  but then  b - a = ~a - ~x = x ..... - b   always exists
*/

void INFERENCES::Aic_Cycle(int opx) {  // only nice loops and solve them
	// first collect tags in loop
	//if(opx==2) h.dp.Image();
	BFTAG elims;
	int npaselim = h.d.SearchEliminations(parentpuz,h.dp, elims);
	if(!npaselim) return; //nothing to find
	h.d.ExpandAll(*parentpuz, h.dp); // 	
	BFTAG xi;
	xb.SetAll_0();
	xi.SetAll_0();// collect tags in loop ok and "to eliminate in true state"
	for(int i = 2; i < parentpuz->col; i++) {
		if(h.d.Is(i, i) && (h.d.Is(i ^ 1, i ^ 1)))
			xb.Set(i);  
		if(h.d.Is(i, i ^ 1) && (!(i & 1)))
			xi.Set(i);
	}
	if(xi.IsEmpty())
		return;  
	if(0 && parentpuz->options.ot) { 
		parentpuz->Image(xi,"candidates potential  eliminations", 0);
	}

	// now check all eliminations for a chain or a cycle    
	for(int i = 2; i < parentpuz->col; i += 2) {
		if(!xi.On(i))
			continue;
		if(0 && parentpuz->options.ot) {
			EE->E("\n\nanalyzing ");
			parentpuz->zpln.ImageTag(i);
			EE->Enl();
		}
		// look for a chain
		if((opx - 2)) //  || (!Op.ocopybugs)) // skip Y chains in serate mode
		{
			BFTAG wch = h.dp.t[i];
			int npasch = wch.SearchChain(h.dp.t, i, i ^ 1);
			if(0 && parentpuz->options.ot) {
				EE->E(" npasch= ");
				EE->Enl(npasch);
			}
			if(!npasch)
				continue; // should never happen
			int ratch = parentpuz->tchain.GetRatingBase((opx == 3) ? 70 : 66, npasch + 1, i >> 1);
			if(ratch) { // chain is accepted load it (more comments in test mode)
				if(parentpuz->options.ot) {
					ExplainPath(wch, i, i^1, npasch + 2, i ^ 1);
				}
				parentpuz->tchain.LoadChain(ratch, "chain", i >> 1);	
			}
		}
		//--------------------------- now look for a loop 
		BFTAG w = h.dp.t[i];
		w &= xb; 
		if(0 && parentpuz->options.ot)
			parentpuz->Image(w,"loop contacts",i);
		if(w.Count() < 2)
			continue; // must have contact with 2 candidates ( false state )
		//in 'X' mode cycle eliminations are done only in the same region
		//first, collect all contacts in a table  and process all pairs
		USHORT tt[100], itt;
		w.String(tt,itt);
		for(int i1 = 0; i1 < itt - 1; i1++) {
			USHORT t1 = tt[i1] ^ 1; // t1 is turned to "true" 
			for(int i2 = i1 + 1; i2 < itt; i2++) {
				USHORT t2 = tt[i2];  // t2 kept to "false"
				if(h.dp.t[t1].On(t2)) {
					//  same cell or region
					// except in 'XY' mode, must be the same digit
					if(opx-3) {
						CANDIDATE candt1 = parentpuz->zpln.zp[t1 >> 1], 
							      candt2 = parentpuz->zpln.zp[t2 >> 1];
						if(candt1.ch-candt2.ch)
							continue; 
					}
					// now we want the xycle crossing t1 and t2.
					// by construction, this is a weak link, never a strong link
					BFTAG wstart;
					wstart.Set(t2);  //we start a search with t1 -> t2
					int npascycle = wstart.SearchCycle(h.dp.t, t1,xb);
					if(!npascycle)
						continue; // could  ben ??
					int rat = parentpuz->tchain.GetRatingBase((opx == 3) ? 70 : 65, npascycle + 1, i >> 1);
					if(!rat)
						continue;// chain is accepted load it (more comments in test mode)
					if(parentpuz->options.ot) {
						ExplainPath(wstart, t1, t1, npascycle + 2, t2);
					}
					parentpuz->tchain.LoadChain(rat, "cycle", i >> 1); 
				}
				else if(opx == 2) // not allowed in 'X' mode and no interest in 'XY'mode
					Aic_Ycycle(t1, t2 ^ 1, xb, i >> 1); // both turned to true 
			}
		}// end for i1 i2
	}// end for i
}

/* we are looking in 'Y' mode for a loop crossing 2 points inside the loop.
   me must find the shortest loop
   that processs can be very long, so it must be designed carefully
   the general idea is the following
   look in priority for a candidate (t1 or t2) with only 2 possible starts in the loop.
   If this exists, take that point as start, if not, 
   take the smallest number of starts and try all of them

   in the search, the strategy is to take the shortest path.
   Doing that, we can kill all possibilities to go back.
   If there is no way back, we try the start from the second candidate in the cell
   
   That process can miss some possibilities, what is accepted.

*/
void INFERENCES::Aic_Ycycle(USHORT t1, USHORT t2, const BFTAG &loop, USHORT cand) {
	// forget if not same digit or t2 is ~t1
	if(cand == (t2 >> 1))
		return;

	CANDIDATE candt1 = parentpuz->zpln.zp[t1 >> 1];
	CANDIDATE candt2 = parentpuz->zpln.zp[t2 >> 1];
	if(candt1.ch - candt2.ch)
		return;

	//USHORT ct1 = (h.dp.t[t1] & loop).Count();
	BFTAG ttt = h.dp.t[t1];
	ttt &= loop;
	USHORT ct1 = ttt.Count();
	if(ct1 < 1)
		return; // should never be
	if(ct1 < 2) {
		Aic_YcycleD(t1, t2 ^ 1, loop, cand);
		return;
	}

	ttt = h.dp.t[t2];
	ttt &= loop;
	USHORT ct2 = ttt.Count();
	if(ct2 < 1)
		return; // should never be

	if(ct2 < 2) {
		Aic_YcycleD(t2, t1 ^ 1, loop, cand);
		return;
	}  
	USHORT wt1 = t1, wt2 = t2;
	if(ct2 < ct1) {
		wt1 = t2;
		wt2 = t1;
	}
	Aic_YcycleD(wt1, wt2 ^ 1, loop, cand);
}

/* now the all possibilities with start t1 and crossing t2
   we must start with a weak link so we exclude as start the strong link
*/
void INFERENCES::Aic_YcycleD(USHORT t1,USHORT t2, const BFTAG &loop,USHORT cand) { // up to 4 starts
	USHORT tt[20], itt, lg = 200;
	PATH resf, resw;
	USHORT tagc = cand << 1, tagcn = tagc ^ 1;

	//(h.dp.t[t1] & loop).String(tt, itt); // starts in table
	BFTAG ttt = h.dp.t[t1];
	ttt &= loop;
	ttt.String(tt, itt); // starts in table

	for(int i = 0; i < itt; i++) {
		if(h.dp.t[tt[i]].On(t1))
			continue; // this is a strong link, skip it
		if(tt[i] == tagcn)
			continue; // don't start with the elimination
		int lw = Aic_Ycycle_start(t1, tt[i], t2, loop, resw);
		if(lw) {
			//if(resw.On(tagcn)) continue; // cand must not be in the loop
			//if(resw.On(tagc)) continue; // to be validated
			if(lw < lg) {
				lg=lw;
				resf = resw;
			}
		}
	}
	// lg final count, resf final result finish the task
	if(lg > 100) {
		Aic_YcycleD2(t1,t2,loop,cand);
		return;
	}// try the second way
	int rat = parentpuz->tchain.GetRatingBase(65, lg, cand);
	if(!rat)
		return;
	// chain is accepted load it and in test mode, find more comments
	if(parentpuz->options.ot) {
		EE->Enl("Y cycle out of the region");
		resf.PrintPathTags(&parentpuz->zpln);
	}
	//  ExplainPath(resf,t1,t1,lg+2,t2);  }
	parentpuz->tchain.LoadChain(rat, "Y cycle", cand); 
}

/* first process failed
   try starttin from the second candidate in the cell containing t1
*/

void  INFERENCES::Aic_YcycleD2(USHORT t1x, USHORT t2x, const BFTAG & loop, USHORT cand)// up to 4 starts
{if(0) {EE->E("Aic_Ycycle d2"); 
         EE->E(" t1x=");parentpuz->zpln.ImageTag(t1x);
         EE->E(" t2x"); parentpuz->zpln.ImageTag(t2x); 
         EE->E(" cand="); parentpuz->zpln.Image(cand);EE->Enl();
		 parentpuz->Image(h.dp.t[t1x^1],"liens",0);EE->Enl();
        }
USHORT t2=t2x^1,t1=0; // new target is  "on"
 USHORT tt[20],itt,lg=200;   PATH resf,resw;
 h.dp.t[t1x^1].String(tt,itt); // starts in table
 if(itt<1) return;// should never be
 t1=tt[0];
 if(0) {EE->E("Aic_Ycycle d2 go"); 
         EE->E(" t1=");parentpuz->zpln.ImageTag(t1);EE->Enl();
        }
 USHORT tagc=cand<<1,tagcn=tagc^1;

 //(h.dp.t[t1] & loop).String(tt,itt); // starts in table
 BFTAG ttt = h.dp.t[t1];
 ttt &= loop;
 ttt.String(tt,itt); // starts in table

 for(int i=0;i<itt;i++) 
  {if(h.dp.t[tt[i]].On(t1)) continue; // this is a strong link, skip it
   if(tt[i]==tagcn) continue; // don't start with the elimination
   int lw=Aic_Ycycle_start(t1,tt[i],t2,loop,resw);
   if(lw)
    {//if(resw.On(tagcn)) continue; // cand must not be in the loop
     //if(resw.On(tagc)) continue; // to be validated
     if(lw<lg){lg=lw;resf=resw;}
    }
  }
 // lg final count, resf final result finish the task
 if(lg>100) return; // should never happen
 int rat=parentpuz->tchain.GetRatingBase(65,lg,cand);
 if(!rat) return;
     // chain is accepted load it and in test mode, find more comments
 if(parentpuz->options.ot){EE->Enl("Y cycle out of the region");
           resf.PrintPathTags(&parentpuz->zpln);   }
	     //  ExplainPath(resf,t1,t1,lg+2,t2);  }
 parentpuz->tchain.LoadChain(rat,"Y cycle",cand); 
}

/* process one start  t1->t1a looking for t1
   and get back the count
   go first to t2
   then track back the path to clean the "done" filtering the process
   and continue to t1. send back the count and the BFTAG
*/
int INFERENCES::Aic_Ycycle_start(USHORT t1, USHORT t1a, USHORT t2, const BFTAG & loop, PATH & path) {
	if(0) {
		EE->E("Aic_Ycycle_start"); 
		EE->E(" start=");
		parentpuz->zpln.ImageTag(t1);
		EE->E(" thru=");
		parentpuz->zpln.ImageTag(t2); 
		EE->E(" first=");
		parentpuz->zpln.ImageTag(t1a);
		EE->Enl();
	}
	BFTAG wstart;
	wstart.Set(t1a);  //we start a search with t1 -> t1a
	int npascycle = wstart.SearchCycleChain(h.dp.t, t1, t2, loop);
	if(!npascycle)
		return 0;
	path.ipth = npascycle + 2;  // itt set to last tag in the path
	if(wstart.TrackBack(h.dp.t, t1, t2, path.pth, path.ipth, t1a))
		return 0; // bug collecting the path 

	if(0) {
		EE->Enl("Aic_Ycycle_start end of first track back"); 
		path.PrintPathTags(&parentpuz->zpln);
		EE->Enl();
	}

	// we go back with a reduced loop, restarting from t2

	BFTAG wstart2;
	wstart2.Set(t2);  //we start next  search from t2 
	BFTAG loop2(loop); // we cancel the forward path in the loop
	for(int i = 1; i < path.ipth; i++)
		loop2.Clear(path.pth[i]); // BFTAG equivalent to tt

	int npas2 = wstart2.SearchCycleChain(h.dp.t, t2, t1, loop2); // and now we continue to the end

	if(0) {
		EE->E("Aic_Ycycle_start after  second npas2=");
		EE->Enl(npas2);
		parentpuz->Image(loop2,"loop2",0);
		EE->Enl();
	}

	// if npas2=0 it can be due to the fact that the first path lock the way back
	// we then try a start from the other candidate in the cell containing t1

	if(!npas2)
		return 0;
	PATH path2;
	path2.ipth = npas2 + 1;  // itt set to last tag in the path
	if(wstart2.TrackBack(h.dp.t, t2, t1, path2.pth, path2.ipth, t1))
		return 0; // bug collecting the path 

	if(0) {
		EE->Enl("Aic_Ycycle_start end of second track back"); 
		path2.PrintPathTags(&parentpuz->zpln);
		EE->Enl();
	}

	// expand path1 with path2
	for(int i = 1; i < path2.ipth; i++)
		path.Add(path2.pth[i]);
	return (path.ipth - 1);  
}

/* we have found something in a forward step
  in test mode, we have to publish an equivalent path
  */
void INFERENCES::ExplainPath(BFTAG & forward, int start, int end, int npas, USHORT relay) {
	if(npas > 40) {
		EE->E("path too long in Explainpath npas=");
		EE->Enl(npas);
		return;
	}

	USHORT tt[50], itt = npas; 
	forward.TrackBack(h.dp.t, start, end, tt, itt, relay);
	parentpuz->zpln.PrintImply(tt, itt);
}
/* done when higher rating  already found
   just eliminate candidates (true) without looking for loops
   replacing X Y and XY search
   */
int INFERENCES::Fast_Aic_Chain() {
	int ir=0;
	parentpuz->TaggingInit();
	parentpuz->zpln.CellLinks();
	parentpuz->zpln.RegionLinks(1); 	         
	h.d.ExpandAll(*parentpuz, h.dp); // 	
	for(int i = 2; i < parentpuz->col; i += 2)
		if(h.d.Is(i, i ^ 1)) {
			parentpuz->zpln.Clear(i >> 1); 	// just clear it if not test mode
			ir++;
			if(0 && parentpuz->options.ot) {
				EE->E("clear ");
				parentpuz->zpln.ImageTag(i);
				EE->Enl();
			}
			BFTAG wch = h.dp.t[i];
			int npasch = wch.SearchChain(h.dp.t, i, i ^ 1);
			ExplainPath(wch, i, i ^ 1, npasch + 2, i ^ 1);
		}
	return ir;
}


/*    <<<<<< SETS_BUFFER  SET   SETS    >>>>>>>>>>>>>

 storing and managing sets of candidates

 sets are "basic" (cell, region) or "event" 
*/



void SETS_BUFFER::GetSpace(USHORT *(& ps),int n) {
	ps = &zs[izs];
	izs += n;
	if(izs >= setsbuffer_lim) {
		ps=0;
		parentpuz->Elimite("SETS_BUFFER");
	}
}

void SET::Image(PUZZLE * parentpuz, FLOG * EE) const {  // liste of candidates in the set
	if(!parentpuz->options.ot)
		return;
	EE->E(type);
	EE->E(" set: ");
	int lim = ncd;
	if(type)
		lim--;
	for(int i = 0; i < lim; i++) {
		parentpuz->zpln.Image(tcd[i]);
		EE->Esp();
	}
	if(type){
		EE->E(tcd[lim]);
		parentpuz->tevent.t[tcd[lim]-event_vi].ImageShort(parentpuz,EE);
	}
}

int SET::Prepare (PUZZLE * parentpuz,USHORT * mi,USHORT nmi,SET_TYPE ty,USHORT ixe) {
	 parentpuz->zcxb.GetSpace(tcd, nmi);
	 if(tcd == 0)
		 return 0;// limite atteinte
	 for(int i = 0; i < nmi; i++)
		 tcd[i] = mi[i]; 
	 ncd = nmi;
	 ix = ixe;
	 type = ty;
	 return 1;
 }




void SETS::Init() {
	izc = 1;
	parentpuz->zcxb.Init();
	nmmax = 0;
	nmmin = 10;
	direct = 0;
}

void SETS::Image() {
	EE->E("\nsets Image izc=");
	EE->E(izc);
	EE->E(" buffer used=");
	EE->Enl((int)parentpuz->zcxb.izs );
	for(int i = 1; i < izc; i++) {
		zc[i].Image(parentpuz,EE);
		EE->Enl();
	}
}


int SETS::ChargeSet (USHORT * mi,USHORT nmi,SET_TYPE ty) {
	if(nmi < 2 || parentpuz->stop_rating)
		return 0;
	if(ty &&  nmi > (chx_max + 1))
		return 0;
	if(!zc[0].Prepare(parentpuz, mi, nmi, ty, izc))
		return 0;
	if(izc < sets_lim) {
		zc[izc++] = zc[0];  
		if(nmi > nmmax)
			nmmax = nmi;  
		if(nmi < nmmin)
			nmmin = nmi;
		return 1;
	}
	parentpuz->Elimite("ZCX");
	return 0;
}

//int SETS::CopySet (int i)
//{if(izc<sets_lim) {zc[izc++]=zc[i];  return 1;}
//parentpuz->Elimite("ZCX");return 0;}

 // multi chains version
int SETS::Interdit_Base80() {
	t= parentpuz->zcf.h.d.t;
	int ir=0;
	for (int ie=1;ie<izc;ie++)     // all active sets 
	{if(zc[ie].type-SET_base) continue;
	int n=zc[ie].ncd; USHORT *tcd=zc[ie].tcd ; 
	BFTAG tbt; tbt.SetAll_1();
	for(int  i=0;i<n;i++)  tbt &= t[tcd[i]<<1];

	if(tbt.IsNotEmpty()) // candidate(s) to clear found
	{if(parentpuz->options.ot && 1){EE->E(" eliminations found in multi chain mode pour ");
	zc[ie].Image(parentpuz,EE);EE->Enl();} 

	for(int j = 3; j < parentpuz->col; j += 2)
		if(tbt.On(j)) // all tags assigned
	{int tot_length=0; USHORT jj=j^1;// skip from assigned to eliminated
	if(parentpuz->options.ot && 0){EE->E(" Set killing "); parentpuz->zpln.ImageTag(jj); EE->Enl(); }
	if(parentpuz->ermax>85+n-3) // gofast if already far above
	{parentpuz->zpln.Clear(jj>>1); ir++;
	if(parentpuz->options.ot){EE->E(" Set fast killing "); parentpuz->zpln.ImageTag(jj); EE->Enl();}
	continue;}
	for(int i2=0;i2<n;i2++)  
	{   BFTAG wch=parentpuz->zcf.h.dp.t[jj]; 
	USHORT end=(tcd[i2]<<1)^1;
	if(wch.On(end))// this is a direct
	{tot_length+=2; continue;}
	int npasch=wch.SearchChain(parentpuz->zcf.h.dp.t,jj,end);
	if(!npasch) EE->Enl(" 0 partial length "); // debugging message
	tot_length+=npasch+2;
	}
	int ratch=parentpuz->tchain.GetRatingBase(80,tot_length,jj>>1);
	if(ratch) // chain is accepted load it (more comments in test mode)
	{// in test mode  give the details for the chains
		// in that case, do it again and print
		if(parentpuz->options.ot)for(int i2=0;i2<n;i2++) 
		{  BFTAG wch=parentpuz->zcf.h.dp.t[jj]; 
		USHORT end=(tcd[i2]<<1)^1;
		if(wch.On(end))// this is a direct
		{USHORT tt[2],itt=2; tt[0]=jj; tt[1]=end; 
		parentpuz->zpln.PrintImply(tt,itt);continue;}
		int npasch=wch.SearchChain(parentpuz->zcf.h.dp.t,jj,end);
		USHORT tt[50],itt=npasch+2; 
		wch.TrackBack(parentpuz->zcf.h.dp.t,jj,end,tt,itt,end);
		parentpuz->zpln.PrintImply(tt,itt);
		}
		parentpuz->tchain.LoadChain(ratch,"chain",jj>>1);	
	}
	} // end  for j
	} // end if
	}// end ie
	return ir;
}
 
/* look for new weak links from sets
   in dynamic mode with part of the tags éliminated (allsteps)
   return code is o if nothing has been done
*/

/// en analyse

int SETS::DeriveDynamicShort(BFTAG & allsteps,SQUARE_BFTAG & dpn,SQUARE_BFTAG & dn){
	if(0)
	     parentpuz->Image(allsteps,"start derive allsteps",0); ///

	dn.ExpandAll(*parentpuz, dpn);
	allparents.AllParents(*parentpuz, dn);

	int ret_code=0;
	t = dn.t;
	for(int ie = 1; ie < izc; ie++) {
		SET chx=zc[ie];
		if(zc[ie].type- SET_base) break; // closed after SET_base
		 // find the reduced set using allsteps
    	USHORT * tcdo = chx.tcd, nno = chx.ncd ,
			     tcd[10],nni=0,aig=0;
		for(int i = 0; i < nno; i++){
			USHORT j=tcdo[i]<<1;
			if(allsteps.On(j)){
				aig=1;  // assigned nothing to do
				break;
			}
			if(allsteps.Off(j^1))
				tcd[nni++]=j;
		}
		if(0 ){
			EE->E( "final count" );
			EE->E(nni);
			EE->E(" for set set"); chx.Image(parentpuz,EE);
			EE->E( " aig =" );
			EE->Enl(aig);
		}
		if(aig || nni<2) 
			continue; // assigned or equivalent
		   // now the set has more than 1 valid candidates
		BFTAG tcf2, tcf2f, bfset;  
		// bfset is the set itself in bf form for the exclusion of the set itself
		for(int i = 0; i < nni; i++)
			bfset.Set(tcd[i] );

		tcf2.SetAll_1();
		for(int i = 0; i < nni; i++) {
			tce[i] = allparents.t[(tcd[i]) ^ 1];
			tce[i] -= bfset;
		}
		for(int i = 0; i < nni; i++) {
			USHORT tg=tcd[i];
			tcf2f = tcf2;
			if(i < nni) {
				for(int k = i + 1; k < nni; k++) {
					tcf2f &= tce[k];
				}
			}
			USHORT ind[640], maxInd,    // raw table
				   ind2[640],maxInd2=0;   // same not forcing a true
			tcf2f.String(ind, maxInd);
			for(int j = 0; j < maxInd; j++) { //clear tags forcing a true
				USHORT tgo=ind[j];
				BFTAG ww(dn.t[tgo]); ww&=bfset;
				if(ww.IsEmpty())
					ind2[maxInd2++] =tgo;
				else
					tcf2f.Clear(tgo);// just for debugging
			}
			if(0 &&maxInd2){///
				EE->E("target"); parentpuz->zpln.ImageTag(tg);
				EE->E(" derive actif"); chx.Image(parentpuz,EE);
				EE->Enl();
				parentpuz->Image(tcf2f," coming from ",0);
				EE->Enl();

			}

			for(int j = 0; j < maxInd2; j++) {
				USHORT tgo=ind2[j];
				if(dn.t[tgo].Off(tg)){
					dpn.t[tgo].Set(tg);
					ret_code=1;
				}
			}
			tcf2 &= tce[i];
			if(tcf2.IsEmpty())
				break; 
		}// end i	

	}	
	return ret_code;
}

void SETS::Derive(int min,int max,int maxs) {
	maxs; // adjust to add the event pointer
	if(max > nmmax)
		max = nmmax;
	if(min < nmmin)
		min = nmmin;
	if(maxs > nmmax)
		maxs = nmmax;
	int maxt = (max > maxs) ? max : maxs;

	if(parentpuz->options.ot && 0) {
		EE->E("debut Derive izc= ");
		EE->E(izc);
		EE->E("  direct= ");
		EE->E(direct);
		EE->E("  min= ");
		EE->E(min);
		EE->E("  max= ");
		EE->E(max);
		EE->E("  maxs= ");
		EE->Enl(maxs);
	}  

	if(direct) {
		t = parentpuz->zcf.h.dp.t;
		allparents.AllParents(*parentpuz, parentpuz->zcf.h.dp);
	}
	else {
		t = parentpuz->zcf.h.d.t;
		allparents.AllParents(*parentpuz, parentpuz->zcf.h.d);
	}// usually direct=0
	for(int ie = 1; ie < izc; ie++) {
		int nnm = zc[ie].ncd;   
		switch(zc[ie].type) {
		case SET_base:
			if(nnm <= max)
				DeriveBase(zc[ie]);
			break;
		case SET_set:
			if(nnm <= (maxs+1))  // +1 for the event pointer
				DeriveSet(zc[ie]);
			break;	
		}   
	}
}
void SETS::DeriveBase(const SET & chx) { // each candidate can be the target
	if(0) {
		EE->E("on traite");
		chx.Image(parentpuz,EE);
		EE->Enl();
	}
	USHORT * tcd = chx.tcd, nni = chx.ncd ; 
	BFTAG tcf2, tcf2f, tcft, bfset;
	tcf2.SetAll_1();
	tcft = tcf2;
	// bfset is the set itself in bf form for the exclusion of the set itself
	for(int i = 0; i < nni; i++)
		bfset.Set(tcd[i] << 1);
	for(int i = 0; i < nni; i++) {
		tce[i] = allparents.t[(tcd[i] << 1) ^ 1];
		tce[i] -= bfset;
	}
	for(int i = 0; i < nni; i++) {
		tcf2f = tcf2;
		if(i < nni) {
			for(int k = i + 1; k < nni; k++) {
				tcf2f &= tce[k];
			}
		}
		USHORT ind[640], maxInd; //v 1 by MD
		tcf2f.String(ind, maxInd);
		for(int j = 0; j < maxInd; j++) {
			if(parentpuz->zcf.IsStart(ind[j], tcd[i] << 1))
				continue; // skip if defined		    
			parentpuz->zcf.LoadDerivedTag(ind[j], tcd[i]);
		}

		tcf2 &= tce[i];
		if(tcf2.IsEmpty())
			return;
	}// end i
}

/* deriving a set creating an event
   the set must be in a dead state for a candidate in left to right mode
   then, the event is established and the event process is called
*/
void SETS::DeriveSet(SET & chx) { // only the "event" can be the target
	USHORT *tcd = chx.tcd, nni = chx.ncd - 1 ; 
	BFTAG tcft, bfset;
	tcft.SetAll_1();
	// bfset is the set itself in bf form for the exclusion of the set itself
	for(int i = 0; i < nni; i++)
		bfset.Set(tcd[i] << 1);
	for(int i = 0; i < nni; i++) {
		tce[i] = allparents.t[(tcd[i] << 1) ^ 1];
		tce[i] -= bfset;
	}
	for(int i = 0; i < nni; i++)
		tcft &= tce[i];

	if(tcft.IsNotEmpty()) { // event established for any 'true' in tcft
		for(USHORT j = 2; j < parentpuz->col; j++) {
			if(tcft.On(j)) {
				if(parentpuz->tevent.EventSeenFor(j, tcd[nni])) { 
					if(!parentpuz->options.ot) {// this just for diag
						EE->E("diag choix");
						chx.Image(parentpuz, EE);
					}
				}
			}
		}// end j
	}// end IsNotEmpty
}

/*      <<<<<<<<<< SQUARE_BFTAG >>>>>>>>>>>>>>>>


*/

void SQUARE_BFTAG::Parents(USHORT x) {
	parents.SetAll_0();
	USHORT tt[640],itt;
	t[x].String(tt,itt);
	for(int i=0;i< itt;i++)
			parents.Set(tt[i]);
	
}

/* the key routine expanding pieces of path out of a->b elementary components
   ExpandAll is the fast mode using in bloc what has already been done  
   partial mode are found in the BFTAG functions
   */ 

void SQUARE_BFTAG::ExpandAll(const PUZZLE &puz, SQUARE_BFTAG & from) {
	(*this) = from; // be sure to start with the set of primary data
	BFTAG t1, t2;
	USHORT p[640], np;
	for(int i = 2; i < puz.col; i++) {
		t[i].String(p, np);
		while(np) {
			t2 = t[i];
			for(int j = 0; j < np; j++) {
				if(p[j] == i)
					continue;
				t[i] |= t[p[j]];
			} // j
			t1 = t[i]; //all bits
			t1 -= t2; //bits set on this pass = all bits excluding bits set on the previous passes
			t1.Clear(i);
			t1.String(p, np);
		}
	}
}// end i   proc

/* that process should be reviewed with rating 85 (and others??)
   060000080001000700000403000003050200090000030008070500000609000005000100020000060 ED=9.0/1.2/1.2;0.104s
   that puzzle is rated 8.8 with that code
   but rated 9.xx if String is outside the while loop
   to have a progrssive well controled action
   String should be outside the while loop

*/


void SQUARE_BFTAG::ExpandShort(const PUZZLE &puz, SQUARE_BFTAG & from ,int npas)
{(*this)=from; // be sure to start with the set of primary data
 for( int i=2;i< puz.col;i++)
  {	int n=1,pas=0;
    while(n && (++pas<npas)) {
	  USHORT p[640], np;
      t[i].String(p, np);
 	   n=0;
	   for(int jx=0;jx<np;jx++){ 
		   int j=p[jx];
		   if((j-i) && t[i].On(j)) {
			   BFTAG x(from.t[j]);
			   if(x.substract(t[i])) {
				   t[i] |= x;
				   n++;
			   }
		   }
	   }
	} // end j  while
  } 
}// end i   proc

/* that table is prepared for the derivation of weak links
   the "from" table is the table of implications
   */
void SQUARE_BFTAG::AllParents(const PUZZLE &puz, const SQUARE_BFTAG & from) {
	t[0].SetAll_0();
	for(int i = 1; i < puz.col; i++)
		t[i] = t[0];
	for(int i = 2; i < puz.col; i++) {
		USHORT ind[640], maxInd; //v 1 by MD, 7x speed
		from.t[i].String(ind, maxInd);
		for(int j = 0; j < maxInd; j++) {
			t[ind[j]].Set(i);
		}
	}
}


/* select tags having a chance to have the shortest path
   this is a filter for the contradiction chains of the form
   x -> ~a  and   ~x -> ~a
   for each tag set to 1 in "x" 
     compute the total of steps needed to get it with both starts
	 keep only tags below  minimal total + n (2) 
   */
void SQUARE_BFTAG::Shrink(BFTAG & x,USHORT it)
{USHORT tt[300],itt,     // tags in table
        ntt[300],min=200,j,lim,ir; 
 x.String(tt,itt);  // put tags in table	
 for(int i=0;i<itt;i++) {
    j=tt[i];
    lim=min+1;
    ir=ExpandToFind(it,j,lim)+ExpandToFind(it^1,j,lim);
    ntt[i]=ir;
    if(ir<min) min=ir;
 }
 lim=min+1;
 for(int i=0;i<itt;i++) {
    if(ntt[i]> lim)
      x.Clear(tt[i]);
 }
}
/* subroutine for Shrink
*/

int SQUARE_BFTAG::ExpandToFind(USHORT td,USHORT tf,USHORT lim){
  BFTAG tagw(t[td]);  // tag to expand
  int npas=0;
  while(1){  // endless loop till tf found or no more expansion or min +2 passed
	if(tagw.On(tf)) return npas;
    if(npas++>lim) return 100; // 
         // now expand one more step
	int n=0; // to check whether something is done
	USHORT p[640], np;
    tagw.String(p, np);
	for(int jx=0;jx<np;jx++){
		int j=p[jx];
        BFTAG x(t[j]);
	    if(x.substract(tagw)) { 
			tagw |= x; 
			n++;
		}
	}
   if(!n) return 100;
  }
}

/* That process is valid only if no derived weak link has been produced
   The search finds the lot of shortest eliminations same length
   return 0 if nothing
   return the length if elimination(s) have been found

   from is the SQUARE_BFTAG of elementary weak links
   elims is set to 1 for all tags found in that process

   only "true" state of non valid candidates are expanded
*/

int SQUARE_BFTAG::SearchEliminations(PUZZLE * parentpuz,SQUARE_BFTAG & from, BFTAG & elims) {
	int npas=0, aigt=0;
	elims.SetAll_0();
	(*this) = from; // be sure to start with the set of primary data
	while(1) {
		int aig=1; // to detect an empty pass
		npas++;
		for(int i = 2; i < parentpuz->col; i += 2)  // only "true" state
			if(parentpuz->zpln.candtrue.Off(i >> 1) &&  // candidate not valid
				t[i].IsNotEmpty()               // should always be
				)
			{
				for(int j = 2; j < parentpuz->col; j++)
					if((j - i) && t[i].On(j)) {
						BFTAG x=from.t[j];
						if(x.substract(t[i])) {
							t[i]|=x;
							aig=0;
						}
					}
					if(t[i].On(i ^ 1)) { // an elimination is seen
						elims.Set(i);
						aigt=1;
					}
			}   
			if(aigt) return npas; // eliminations found
			if(aig) return 0;     // empty pass
	}// end while
}

/*               <<<<<<<   TCANDGO  >>>>>>>>>>>>>
                 <<<<<<< CHAINSTORE   >>>>>>>>>>
 storage in nested mode 
 strongg links
 nested chains
*/



void TCANDGO ::AddStrong(USHORT k1, USHORT k2, const BFTAG &bf, USHORT cpt) {
	if(its >= 598)
		return;
	ts[its++].Add(k1, k2, bf, cpt);
	ts[its++].Add(k2, k1, bf, cpt);
}


const CANDGOSTRONG * TCANDGO ::Get(USHORT t1, USHORT t2) const {
	USHORT c1 = t1 >> 1, c2 = t2 >> 1;
	for(int i = 1; i < its; i++) {
		const CANDGOSTRONG * w = &ts[i];
		if(w->key1 - c1)
			continue;
		if(w->key2 - c2)
			continue;
		return w;
	}
	return 0;
}


/* storage of the nested chains for pring purpose
  storing is done in a buffer with a double index
  primary for a chain (start,number=
  secondary for a nested object: start index, end index
  */
  
//	CHAINSTORE(PUZZLE * parent){parentpuz=parent;}

void CHAINSTORE::Init() {
	ibuf=0;
	starts[0] = ends[0] = 0;
	ise = 1; 
	s2[0] = 0;
	e2[0] = 0;
	ise2 = 1;
} // 0 is "empty"


USHORT CHAINSTORE::AddChain(USHORT * tch, USHORT n) {
	if(n>70)n=70;//don't store more than 70 not realistic
	starts[ise] = ibuf;

	if((ibuf + n+2 )< Size_Store)  {
		for(int i = 0; i < n; i++)
			buf[ibuf++] = tch[i];
	}
	ends[ise] = ibuf;
	if(ise >= 5000)
		return ise; // don't pass the limit
	else
		return ise++;
}


USHORT CHAINSTORE::AddOne(USHORT * tch, USHORT n) {
	if(ise2 >= 2000)
		return 0;
	s2[ise2] = e2[ise2] = AddChain(tch, n);
	return ise2++;
}


USHORT CHAINSTORE::AddMul(USHORT d, USHORT f) {
	if(ise2 >= 2000)
		return 0;
	s2[ise2] = d;
	e2[ise2] = f;
	return ise2++;
}

/* called by GoBackNested to count only once 
   chains already used

   chain(s) pointed by index are in use
   look to see if they are already thers.
   if yes return 0,
   if not, store them and return 1; 
*/
int CHAINSTORE::Use(CHAINSTORE & store_source,USHORT index){
	if(index>=store_source.ise2  || index<1){
		return 2;  // default is count it
	}
	int id = store_source.s2[index], ie = store_source.e2[index],
		 nchains=ie-id+1;
	if(nchains<=0) // default is count
		return 3; 
	  // loop on stored chains with that count
	for(int myindex = 1; myindex <= ise2; myindex++) {
		int myid = s2[myindex], myie = e2[myindex],
			mynchains=myie-myid+1;
		if(mynchains-nchains)
			continue;
		// now compare the individual paths 
		int aigok=1;
		for(int i = id,myi=myid ; i <= ie; i++,myi++) {
			const USHORT * tx = &store_source.buf[store_source.starts[i]];
			const USHORT * mytx = &buf[starts[myi]];

			USHORT 	n = store_source.ends[i] - store_source.starts[i];
			USHORT 	myn = ends[myi] - starts[myi];
			if(n<=0) 
				return 4; // set default return 1 if anomaly
			if(n -myn) {
				aigok=0; 
			}
			else{
				for(int j=0;j<n;j++){
					if(tx[j]-mytx[j]){
						aigok=0;
						break;
					}
				}
			}
			if(!aigok)
				break;
		}
		if(aigok)
			return 0;
	}
	// nothing matches, store it and return 1;
	int idn,ien;
	for(int i = id; i <= ie; i++) {
		USHORT * tx = &store_source.buf[store_source.starts[i]];
		USHORT 	n = store_source.ends[i] - store_source.starts[i];		
		ien=AddChain(tx,n);
		if(i==id)
			idn=ien;
	}
	AddMul(idn,ien);
	return 100+ise2;
}




void CHAINSTORE::Print(PUZZLE * parentpuz, FLOG * EE,USHORT index) const {
	if(index>=ise2  || index<1){
		if(!index)
			EE->Enl("\n\nchainstore::print  index null not stored");
		else{
			EE->E("\n\nchainstore::print invalid entry index=");
			EE->Enl( index);
		}
		return;
	}
	int id = s2[index], ie = e2[index];
	for(int i = id; i <= ie; i++) {
		const USHORT * tx = &buf[starts[i]];
		USHORT 	n = ends[i] - starts[i];
		if(n>70) {
			EE->Enl("length too high forced to 5");
			EE->E("index ="); EE->E(index);
			EE->E( " id="); EE->E(id); 
			EE->E( " ie="); EE->E(ie); 
			EE->E( " i="); EE->E(i); 

			EE->E( " ends[i]="); EE->E(ends[i]); 
			EE->E( " starts[i]="); EE->Enl(starts[i]); 
			n=5;
		}
		if(n > 0)
			parentpuz->zpln.PrintImply(tx, n);
	}
}

