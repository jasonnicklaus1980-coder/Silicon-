#include "mpc_plugin.h"
#include "ymfm_opm.h"
#include "ymfm_opn.h"
#include "ymfm_opl.h"
#include <algorithm>
#include <cmath>
class Engine : public mpc::Plugin {
 ymfm::ymfm_interface iface; ymfm::ym2151 chip{iface}; ymfm::ym2164 opp{iface};
 ymfm::ym2612 genesis{iface}; ymfm::ymf262 opl{iface}; int engine=-1; double hostRate=44100;
 void gw(int c,int a,int v){genesis.write((c>=3?2:0),a);genesis.write((c>=3?3:1),v);}
 void ow(int a,int v){opl.write(a>=256?2:0,a&255);opl.write(a>=256?3:1,v);}
 int op(int c){return (c/3)*8+c%3;}
 struct Voice {int note=-1,channel=-1; unsigned age=0;}; Voice voices[8]; int oplPitch[8]{}; unsigned age=0;
 double ratio=1,phase=0; float prev[2]{},next[2]{}; int patch=-1; float delay[32768][2]{}; unsigned cursor=0; float lp[2]{};
 void wr(int a,int v){auto& target=engine==3?static_cast<ymfm::ym2151&>(opp):chip;target.write_address(a);target.write_data(v);}
 void off(int c){if(engine==1){if(c<6)gw(0,0x28,c+(c>=3));}else if(engine==2)ow(0xb0+c,oplPitch[c]);else wr(8,c);voices[c].note=-1;}
 void configure(int p){if(engine==1){for(int c=0;c<6;c++){off(c);int k=c%3;gw(c,0xb0+k, (p==1?0x34:0x24));gw(c,0xb4+k,0xc0);for(int o=0;o<4;o++){int x=k+4*o;gw(c,0x30+x,o==0?2:1);gw(c,0x40+x,o<2?35:15);gw(c,0x50+x,p==3?12:31);gw(c,0x60+x,10);gw(c,0x70+x,4);gw(c,0x80+x,0x58);}}patch=p;return;}if(engine==2){ow(0x105,1);for(int c=0;c<8;c++){off(c);int x=op(c);ow(0x20+x, p==2?3:2);ow(0x23+x,1);ow(0x40+x, p==1?12:28);ow(0x43+x,8);ow(0x60+x,p==3?0x48:0xf8);ow(0x63+x,p==3?0x48:0xf8);ow(0x80+x,0x76);ow(0x83+x,0x76);ow(0xc0+c,0x34);}patch=p;return;}for(int c=0;c<8;c++){off(c);wr(0x20+c,0xc0| (p==2?5:4) | (p==1?0x20:0));for(int o=0;o<4;o++){int x=c+8*o;wr(0x40+x,o==0?(p==2?3:2):1);wr(0x60+x,o<2?(p==1?20:38):12);wr(0x80+x,p==3?12:31);wr(0xa0+x,o<2?13:9);wr(0xc0+x,p==3?0:4);wr(0xe0+x,p==3?0x24:0x58);}}patch=p;}
 void midi(const mpc::MidiEvent& e){if(e.isAllNotesOff()){for(int c=0;c<8;c++)if(voices[c].channel==e.channel())off(c);}else if(e.isNoteOff()){for(int c=0;c<8;c++)if(voices[c].note==e.note()&&voices[c].channel==e.channel())off(c);}else if(e.isNoteOn()){int c=0;for(int j=0;j<(engine==1?6:8);j++){if(voices[j].note<0){c=j;break;}if(voices[j].age<voices[c].age)c=j;}off(c);voices[c]={e.note(),e.channel(),++age};if(engine==1 || engine==2){double hz=440*std::pow(2.,(e.note()+detune-69)/12.);int b=0;double f=hz*(engine==1?288.*1048576/7670454:288.*1048576/14318180);while(f>(engine==1?2047:1023)&&b<7){f/=2;b++;}int fn=std::clamp(int(f+0.5),0,engine==1?2047:1023);if(engine==1){int k=c%3;gw(c,0xa4+k,(b<<3)|(fn>>8));gw(c,0xa0+k,fn&255);gw(c,0x4c+k,12+(127-e.velocity())/4);gw(0,0x28,0xf0| (c+(c>=3)));}else{ow(0x43+op(c),8+(127-e.velocity())/4);ow(0xa0+c,fn&255);oplPitch[c]=(b<<2)|(fn>>8);ow(0xb0+c,0x20|oplPitch[c]);}return;}int m=std::clamp(e.note()-13,0,95);const int kc[]={0,1,2,4,5,6,8,9,10,12,13,14};int pitch=std::clamp(int(std::round((e.note()-13+detune)*64)),0,95*64+63); m=pitch/64; wr(0x28+c,((m/12)<<4)|kc[m%12]);wr(0x30+c,(pitch%64)<<2);int att=(127-e.velocity())/4;for(int o=2;o<4;o++)wr(0x60+c+8*o,12+att);wr(8,0x78|c);}}
 void applyOperator(int c){
 if(editOp<0 || editRatio<0)return;
 int o=editOp; int mult=editRatio;
 if(engine==1){int x=c%3+4*o;gw(c,0x30+x,mult);gw(c,0x40+x,editLevel);gw(c,0x50+x,editAttack);gw(c,0x60+x,editDecay);gw(c,0x80+x,(editSustain<<4)|editRelease);}
 else if(engine==2){if(o>1)return;int x=op(c)+3*o;ow(0x20+x,mult);ow(0x40+x,editLevel/2);ow(0x60+x,((editAttack/2)<<4)|(editDecay/2));ow(0x80+x,(editSustain<<4)|editRelease);}
 else {int x=c+8*o;wr(0x40+x,mult);wr(0x60+x,editLevel);wr(0x80+x,editAttack);wr(0xa0+x,editDecay);wr(0xe0+x,(editSustain<<4)|editRelease);}
 }
 void tick(){if(engine==1){ymfm::ym2612::output_data o;genesis.generate(&o);for(int c=0;c<2;c++){prev[c]=next[c];next[c]=o.data[c]/32768.f;}return;}if(engine==2){ymfm::ymf262::output_data o;opl.generate(&o);for(int c=0;c<2;c++){prev[c]=next[c];next[c]=o.data[c]/32768.f;}return;}ymfm::ym2151::output_data out;if(engine==3)opp.generate(&out);else chip.generate(&out);for(int c=0;c<2;c++){prev[c]=next[c];next[c]=out.data[c]/32768.f;}}
public:
 int lastEditOp=-1; int editOp=-1,editRatio=-1,editLevel=12,editAttack=31,editDecay=10,editSustain=5,editRelease=8;
 bool additive=false; float layerGain=1; float detune=0;
 static mpc::PluginInfo info(){mpc::PluginInfo i;i.name="SILICON";i.vendor="Shannon Instruments";i.uniqueId=MPC_FOURCC('S','i','L','1');i.version=1000;i.isInstrument=true;i.numInputs=0;i.numOutputs=2;return i;}
 Engine():mpc::Plugin(info()){addParam(mpc::choiceParam("patch","Tone",{"Soul Keys","Detroit Bass","Crystal Bells","Slow Pad"},0));addParam(mpc::floatParam("level","Volume",0,1,0.35f));addParam(mpc::choiceParam("engine","Engine",{"YM2151","YM2612","YMF262","YM2164"},0));addParam(mpc::floatParam("drive","Drive",0,1,0));addParam(mpc::floatParam("echo","Echo",0,1,0));addParam(mpc::floatParam("tone","Brightness",0,1,1)); const char* names[]={"Soul Keys","Detroit Bass","Crystal Bells","Slow Pad"}; const char* engines[]={"OPM","Genesis","OPL","OPP"}; for(int e=0;e<4;e++)for(int p=0;p<4;p++)for(int v=0;v<2;v++){std::string name=std::string(engines[e])+" "+names[p]+(v?" Space":"");addProgram(name.c_str(),{float(p),0.35f,float(e),v?0.15f:0.f,v?0.28f:0.f,v?0.65f:1.f});}}
 void prepare(double rate,int) override {hostRate=std::max(1.0,rate);reset();}
 void reset() override {chip.reset();opp.reset();genesis.reset();opl.reset();engine=-1;for(auto& v:voices)v=Voice{};age=0;for(auto& x:oplPitch)x=0;phase=0;prev[0]=prev[1]=next[0]=next[1]=0;patch=-1;cursor=0;for(auto& d:delay)d[0]=d[1]=0;lp[0]=lp[1]=0;}
 void process(mpc::ProcessContext& ctx) override {int en=std::clamp(int(get(2)+0.5f),0,3);if(en!=engine){reset();engine=en;ratio=(en==3?opp.sample_rate(3579545):en==0?chip.sample_rate(3579545):en==1?genesis.sample_rate(7670454):opl.sample_rate(14318180))/hostRate;}int p=std::clamp(int(get(0)+0.5f),0,3);if(p!=patch || (lastEditOp>=0 && lastEditOp!=editOp))configure(p);lastEditOp=editOp;if(editOp>=0)for(int c=0;c<(engine==1?6:8);c++)applyOperator(c);int e=0;float gain=get(1),drive=get(3),echo=get(4),tone=get(5); unsigned lag=std::clamp(unsigned(hostRate*0.24),1u,32767u); float alpha=0.025f+0.975f*tone;for(int n=0;n<ctx.numFrames;n++){while(e<ctx.numMidiEvents&&ctx.midi[e].sampleOffset<=n){auto &ev=ctx.midi[e++];midi(ev);if(ev.isNoteOn()&&editOp>=0)for(int c=0;c<(engine==1?6:8);c++)if(voices[c].note==ev.note())applyOperator(c);}phase+=ratio;while(phase>=1){tick();phase-=1;}for(int c=0;c<std::min(2,ctx.numOutputs);c++){float x=(prev[c]+float(phase)*(next[c]-prev[c]))*gain*(engine==2?2.f:1.f);lp[c]+=alpha*(x-lp[c]);x=lp[c];if(drive>0)x=std::tanh(x*(1+4*drive));float d=delay[(cursor+32768-lag)&32767][c];delay[cursor][c]=x+d*0.3f*echo;ctx.outputs[c][n]=(additive?ctx.outputs[c][n]:0.f)+layerGain*(x+echo*d);}cursor=(cursor+1)&32767;}}
};
class Silicon : public mpc::Plugin {
 Engine a,b,c,d; bool active=false; double rate=44100, lfo=0; unsigned pos=0; float chorusBuf[4096][2]{}, rev[4][8192]{}; unsigned rp[4]{}; float eqLow[2]{}, eqHigh[2]{}, compEnv=0; float ap[2][4]{}, fb[2]{};
void addLayerProgram(const char* name,std::vector<float> values){values.insert(values.end(),{2.f,0.f,0.f,3.f,3.f,0.f,0.f,0.f,0.f,0.f});values.insert(values.end(),{0.f,0.f,1.f,12.f,31.f,10.f,5.f,8.f});addProgram(name,std::move(values));}
public:
 Silicon():mpc::Plugin(Engine::info()){
 addParam(mpc::choiceParam("patch","Tone",{"Soul Keys","Detroit Bass","Crystal Bells","Slow Pad"},0));
 addParam(mpc::floatParam("level","Volume",0,1,0.35f));
 addParam(mpc::choiceParam("engine","Engine",{"YM2151","YM2612","YMF262","YM2164"},0));
 addParam(mpc::floatParam("drive","Drive",0,1,0));
 addParam(mpc::floatParam("echo","Echo",0,1,0));
 addParam(mpc::floatParam("tone","Brightness",0,1,1));
 addParam(mpc::choiceParam("layer","Layer Engine",{"YM2151","YM2612","YMF262","YM2164"},1));
 addParam(mpc::choiceParam("layerTone","Layer Tone",{"Soul Keys","Detroit Bass","Crystal Bells","Slow Pad"},3));
 addParam(mpc::floatParam("blend","Layer Mix",0,1,0));
 addParam(mpc::floatParam("chorus","Chorus",0,1,0));
 addParam(mpc::floatParam("reverb","Reverb",0,1,0));
 addParam(mpc::choiceParam("pure","FX Mode",{"FX On","Pure"},0));
 addParam(mpc::floatParam("bass","Bass EQ",-1,1,0));
 addParam(mpc::floatParam("treble","Treble EQ",-1,1,0));
 addParam(mpc::floatParam("compress","Compressor",0,1,0));
 addParam(mpc::floatParam("crush","Bit Crusher",0,1,0));
 addParam(mpc::floatParam("phaser","Phaser",0,1,0));
 addParam(mpc::floatParam("flanger","Flanger",0,1,0));
 addParam(mpc::choiceParam("layer3","Layer 3 Engine",{"YM2151","YM2612","YMF262","YM2164"},2));
 addParam(mpc::choiceParam("layer3Tone","Layer 3 Tone",{"Soul Keys","Detroit Bass","Crystal Bells","Slow Pad"},0));
 addParam(mpc::floatParam("layer3Mix","Layer 3 Mix",0,1,0));
 addParam(mpc::choiceParam("layer4","Layer 4 Engine",{"YM2151","YM2612","YMF262","YM2164"},3));
 addParam(mpc::choiceParam("layer4Tone","Layer 4 Tone",{"Soul Keys","Detroit Bass","Crystal Bells","Slow Pad"},3));
 addParam(mpc::floatParam("layer4Mix","Layer 4 Mix",0,1,0));

 addParam(mpc::floatParam("detune1","Layer 1 Detune",-12,12,0));
 addParam(mpc::floatParam("detune2","Layer 2 Detune",-12,12,0));
 addParam(mpc::floatParam("detune3","Layer 3 Detune",-12,12,0));
 addParam(mpc::floatParam("detune4","Layer 4 Detune",-12,12,0));
 addParam(mpc::choiceParam("editLayer","Edit Layer",{"Off","A","B","C","D"},0));
 addParam(mpc::choiceParam("editOp","Operator",{"1","2","3","4"},0));
 addParam(mpc::floatParam("ratio","Operator Ratio",0,15,1));
 addParam(mpc::floatParam("opLevel","Operator Attenuation",0,127,12));
 addParam(mpc::floatParam("attack","Operator Attack",0,31,31));
 addParam(mpc::floatParam("decay","Operator Decay",0,31,10));
 addParam(mpc::floatParam("sustain","Operator Sustain Attenuation",0,15,5));
 addParam(mpc::floatParam("release","Operator Release",0,15,8));
 const char* names[]={"Soul Keys","Detroit Bass","Crystal Bells","Slow Pad"};
 const char* engines[]={"OPM","Genesis","OPL","OPP"};
 for(int e=0;e<4;e++)for(int p=0;p<4;p++)for(int v=0;v<2;v++){
 std::string name=std::string(engines[e])+" "+names[p]+(v?" Space":"");
 addLayerProgram(name.c_str(),{float(p),.35f,float(e),v?.15f:0.f,v?.28f:0.f,v?.65f:1.f,float((e+1)%4),3.f,0.f,v?.25f:0.f,v?.2f:0.f,0.f,0.f,0.f,0.f,0.f,0.f,0.f});}
 for(int e=0;e<4;e++)for(int p=0;p<4;p++){
 std::string name=std::string("Hybrid ")+engines[e]+" "+names[p];
 addLayerProgram(name.c_str(),{float(p),.35f,float(e),.08f,.22f,.8f,float((e+1)%4),3.f,.35f,.25f,.2f,0.f,0.f,0.f,0.f,0.f,0.f,0.f});}
 for(int e=0;e<4;e++)for(int p=0;p<4;p++)for(int v=0;v<4;v++){
 const char* colors[]={"Warm","Wide","Dust","Dream"};
 std::string name=std::string(engines[e])+" "+colors[v]+" "+names[p];
 addLayerProgram(name.c_str(),{float(p),.30f,float(e),v==2?.28f:0.f,v==3?.35f:.08f,v==0?.45f:.85f,float((e+1+v)%4),float((p+1)%4),v==1?.32f:0.f,v==1?.55f:.12f,v==3?.48f:.15f,0.f,0.f,0.f,0.f,0.f,0.f,0.f});}
 for(int j=0;j<388;j++){
 int e=j%4,p=(j/4)%4,bank=j/16;
 float t=float(bank)/24.f;
 const char* family[]={"Keys","Bass","Bell","Pad"};
 std::string name=std::string(engines[e])+" "+family[p]+" Studio "+std::to_string(bank+1);
 addLayerProgram(name.c_str(),{float(p),.28f+.06f*t,float(e),.04f+.18f*t,.08f+.24f*t,.55f+.4f*t,float((e+1+bank)%4),float((p+bank)%4),bank%3==0?.25f:0.f,.12f+.35f*t,.1f+.3f*t,0.f,.12f*t,-.15f*t,.1f*t,0.f,bank%4==0?.15f:0.f,0.f});
 }
 }
 void prepare(double rate,int frames) override {this->rate=std::max(1.0,rate);a.prepare(rate,frames);b.prepare(rate,frames);c.prepare(rate,frames);d.prepare(rate,frames);reset();}
 void reset() override {a.reset();b.reset();c.reset();d.reset();active=false;pos=0;lfo=0;for(auto& x:chorusBuf)x[0]=x[1]=0;for(auto& x:rev)for(auto& y:x)y=0;for(auto& x:rp)x=0;eqLow[0]=eqLow[1]=eqHigh[0]=eqHigh[1]=compEnv=0;for(auto& ch:ap)for(auto& z:ch)z=0;fb[0]=fb[1]=0;}
 void process(mpc::ProcessContext& ctx) override {
 for(int i=0;i<6;i++){for(auto* e:{&a,&b,&c,&d})e->set(i,get(i));}
 if(get(11)>0.5f){for(auto* e:{&a,&b,&c,&d}){e->set(3,0);e->set(4,0);e->set(5,1);}}
 b.set(0,get(7));b.set(2,get(6));float mix=get(8);
 c.set(2,get(18));c.set(0,get(19));d.set(2,get(21));d.set(0,get(22));
 a.detune=get(24);b.detune=get(25);c.detune=get(26);d.detune=get(27);
 int selected=int(get(28)+.5f);int li=0;for(auto* e:{&a,&b,&c,&d}){e->editOp=(++li==selected)?int(get(29)+.5f):-1;e->editRatio=int(get(30)+.5f);e->editLevel=int(get(31)+.5f);e->editAttack=int(get(32)+.5f);e->editDecay=int(get(33)+.5f);e->editSustain=int(get(34)+.5f);e->editRelease=int(get(35)+.5f);}
 float m3=get(20),m4=get(23),norm=1+m3+m4;
 a.layerGain=(1-mix)/norm;a.additive=false;a.process(ctx);
 b.layerGain=mix/norm;b.additive=true;b.process(ctx);
 c.layerGain=m3/norm;c.additive=true;c.process(ctx);
 d.layerGain=m4/norm;d.additive=true;d.process(ctx);active=true;
 if(ctx.numOutputs<2)return;
 float pm=get(16),fm=get(17); float cm=get(9),rm=get(10);bool pure=get(11)>0.5f;
 float bass=get(12),treble=get(13),comp=get(14),crush=get(15); float al=1-std::exp(-6.2831853*200/rate), ah=1-std::exp(-6.2831853*3500/rate);
 for(int n=0;n<ctx.numFrames;n++){
 float x[2]={ctx.outputs[0][n],ctx.outputs[1][n]};
 for(int c=0;c<2;c++){
 float dry=x[c]; double fd=std::clamp(rate*(.003+.002*std::sin(lfo+c*1.57)),1.,4094.); unsigned fk=unsigned(fd); float frac=fd-fk; float fw=chorusBuf[(pos+4096-fk)&4095][c]*(1-frac)+chorusBuf[(pos+4095-fk)&4095][c]*frac; chorusBuf[pos][c]=dry+(pure?0.f:fm*.35f*fw);double d=std::clamp(rate*(.016+.003*std::sin(lfo+c*3.14159265)),1.,4094.);unsigned k=unsigned(d);float f=d-k;float wet=chorusBuf[(pos+4096-k)&4095][c]*(1-f)+chorusBuf[(pos+4095-k)&4095][c]*f;if(!pure)x[c]=x[c]*(1-.5f*cm)+wet*.5f*cm; if(!pure){x[c]+=fm*.5f*fw; float coeff=float((1-std::tan(3.14159265*(400+300*std::sin(lfo))/rate))/(1+std::tan(3.14159265*(400+300*std::sin(lfo))/rate))); float y=x[c]+.2f*pm*fb[c]; for(auto& z:ap[c]){float out=-coeff*y+z;z=y+coeff*out;y=out;}fb[c]=y;x[c]=x[c]*(1-.5f*pm)+y*.5f*pm;}
 }
 pos=(pos+1)&4095;lfo+=6.2831853*.35/rate;if(lfo>6.2831853)lfo-=6.2831853;
 float w[2]{};const double times[]={.0297,.0371,.0411,.0437};for(int j=0;j<4;j++){unsigned len=std::clamp(unsigned(rate*times[j]),1u,8192u);if(rp[j]>=len)rp[j]=0;float y=rev[j][rp[j]];rev[j][rp[j]]=(x[0]+x[1])*.25f+y*.72f;rp[j]=(rp[j]+1)%len;w[j%2]+=y*.5f;}
 for(int c=0;c<2;c++){x[c]+=pure?0.f:rm*w[c];eqLow[c]+=al*(x[c]-eqLow[c]);eqHigh[c]+=ah*(x[c]-eqHigh[c]);if(!pure){x[c]+=bass*eqLow[c]+treble*(x[c]-eqHigh[c]);if(crush>0){float q=std::exp2(15-11*crush);x[c]=std::round(x[c]*q)/q;}}}
 float peak=std::max(std::abs(x[0]),std::abs(x[1]));compEnv+=(peak>compEnv?.02f:.001f)*(peak-compEnv);float cg=(!pure&&comp>0)?1.f/(1.f+comp*4.f*std::max(0.f,compEnv-.15f)):1.f;
 for(int c=0;c<2;c++)ctx.outputs[c][n]=std::clamp(x[c]*cg,-1.f,1.f);
 }
 }
};
MPC_PLUGIN_ENTRY(Silicon)
