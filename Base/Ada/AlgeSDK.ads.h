// AlgeSDK.ads.h // The high level interface to AlgeSDK in conformance with Ada Specifications
#ifndef ALGESDK_ADS_H
#define ALGESDK_ADS_H
#include <math.h>
#include <vector>
#include <string>
#include <stdio.h>
#include <list>
#include <Box2D/Box2D.h>

using namespace std;
#define GLfloat float
#include "../CCommands.h"
#include "../../Classlets/com/acnodelabs/funkit/CAnimator.hpp"
#include "../../Base/camera.h"

const float FACTOR_RADIANS_DEGREES = 57.295779513082;
void inline aluLookAt(GLfloat eyex, GLfloat eyey, GLfloat eyez,
                      GLfloat centerx, GLfloat centery, GLfloat centerz,
                      GLfloat upx, GLfloat upy, GLfloat upz);

class i2 {
public:
	int x, y;
	void clear() { x = 0; y = 0; }
	i2() { clear(); };
	i2(int mx, int my) { x = mx; y = my; }
	i2 half() { return i2(x/2,y/2); }
	i2 twice() { return i2(x * 2, y * 2); }
};

class f2 {
public:
	float x, y;
	void clear() { x = 0; y = 0; }
	f2() { clear(); };
	f2(float mx, float my) { x = mx; y = my; }
	void CopyFrom(i2 p) { x = float(p.x); y = float(p.y); }
};

class f3 {
public:
    float x,y,z;
	char xyz[64] = { 0 };
	void clear() { x = 0; y = 0; z = 0; }
	f3(float mx, float my, float mz) { x = mx; y = my; z = mz; }
	string str(string fmt) {
    //sprintf_s(xyz, 64, fmt.c_str(), x, y, z);
    sprintf(xyz, fmt.c_str(), x, y, z);
		return string(xyz);
	}
	f2 xy() {return f2{ x,y }; }
	f3* ref() { return this; }

    f3() {clear();};
};

class PosRotScale {
public:
    f3 pos;
    f3 rot;
    float scale = 1.0;
	bool hidden = false;
};

class Serializable : public PosRotScale {
    
    public:
    string UUID;
    //can be used externally for Appends
    void readin(FILE *f) {
        fread((void*)&pos,sizeof(pos),1, f);
        fread((void*)&rot,sizeof(rot),1, f);
        fread((void*)&scale,sizeof(scale),1, f);
    }
    
    void writeout(FILE *f) {
        fwrite((void*)&pos,sizeof(pos),1, f);
        fwrite((void*)&rot,sizeof(rot),1, f);
        fwrite((void*)&scale,sizeof(scale),1, f);
    }
    
    void SetSerializeTag(const char* UUID) {
        this->UUID = string(UUID);
     //   return;
        if (this->UUID.size()>0) {
            FILE* f = fopen(this->UUID.c_str(), "rb");
            if (f) {
                readin(f);
                fclose(f);
            }
        }
    }
    ~Serializable() {
        if (UUID.size()>0) {
            FILE* f = fopen(UUID.c_str(), "wb");
            if (f) {
                writeout(f);
                fclose(f);
                printf("\tSerialized: %s", UUID.c_str());
            }
        }
    }
};


class GameObject : public Serializable {
	
public:
	//Add PRS to multiple drawing of same model
	//use getInstancePtr to access instances
	vector<PosRotScale> prsInstances;
	int modelId;
    ResourceInf* resInf;
    short custom_type;
    GameObject* parent;
    std::list<GameObject*> children;
	short instanceBiengRendered;
    bool rotatefirst;
    string UUID;
    string Text;
   // float Scale;
    bool billboard;
    PosRotScale desirable;
	
	static i2 windowSize;

    CAnimator animPos, animRot, animScale;
	b2Body * phys_body;

    void Update(float dt) {
		
        if (animPos.active) {
            animPos.Step(dt);
            pos.x = mPos[0];
            pos.y = mPos[1];
            pos.z = mPos[2];
        }
        if (animRot.active) {
            animRot.Step(dt);
            rot.x = mRot[0];
            rot.y = mRot[1];
            rot.z = mRot[2];
        }
        if (animScale.active) {
            animScale.Step(dt);
            scale = mScale[0];//3d?
        }
		if (phys_body && phys_body->GetType()==b2_dynamicBody) {
			b2Vec2 position = phys_body->GetPosition();
			float32 angle = phys_body->GetAngle();
			pos.x = position.x ;
			pos.y = position.y ;
			rot.z = angle * 57.272727;
		}
    }
    
    void o2vf(f3* o, float* v3) {
        v3[0] = o->x; v3[1] = o->y; v3[2] = o->z;
    }
    
    
    float tgtPos[3];
    float tgtRot[3];
    float mPos[3];
    float mRot[3];
    float mScale[3];
    float tgtScale[3];
    
	void transitionTo(f2 pos, float speed = 0.3) {
		PosRotScale tgt;
		tgt.pos.x = pos.x;
		tgt.pos.y = pos.y;
		transitionTo(tgt, speed);
	}

    void transitionTo(PosRotScale tgt, float speed = 0.3) {
        o2vf(&tgt.pos, tgtPos);
        o2vf(&tgt.rot, tgtRot);
        o2vf(&pos, mPos);
        o2vf(&rot, mRot);
        mScale[0] = mScale[1] = mScale[2] = scale;
        tgtScale[0]=tgtScale[1]=tgtScale[2]= tgt.scale;
        animPos.Reset(mPos, tgtPos, speed);
        animRot.Reset(mRot, tgtRot, speed);
        animScale.Reset(mScale, tgtScale, speed);
    }
    
    void Clone(GameObject* clone) {
        clone->pos = pos;
        clone->rot = rot;
     //   clone->Scale = Scale;
        clone->scale = scale;
        clone->resInf = resInf;
        clone->custom_type = custom_type;
        clone->modelId = modelId;
        //clone->children = children;
        clone->hidden = hidden;
        clone->rotatefirst = rotatefirst;
        clone->parent = parent;
        clone->billboard = billboard;
    }
    
    void AddChild(GameObject* child){
        children.push_back(child);
        child->parent = this;
    }
	
	void AddInstance(f2 pos) {
		PosRotScale prs;
		prs.pos.x = pos.x;
		prs.pos.y = pos.y;
		prsInstances.push_back(prs);
	}
	PosRotScale* getInstancePtr(int n) {
		if (n < 0) return 0;
		return &prsInstances.at(n-1);
	}
    GameObject() {
        pos.clear();
        rot.clear();
		prsInstances.clear();
        modelId = -1;
        scale = 1.;
        resInf = nullptr;
        custom_type = -1;
        children.clear();
        hidden = false;
        parent = nullptr;
        rotatefirst = true;
        UUID = "";
  //    Scale = 1.0f;
        billboard = false;
    }

    string Name() {
        if (Text.size()) return Text;
		if (!resInf) return "Undefined";
        if (resInf->name.size()) return resInf->name;
        if (resInf) if (resInf->alx.size()) return resInf->alx;
        if (UUID.size()) return UUID;
        return "Undefined";
    }
    
    ~GameObject() {

    }
    
};
i2 GameObject::windowSize;

class FontObject : public GameObject {
public:
    
};


class Camera : public GameObject, public CCamera {
	short mode;

public:
	int windowWidth;//for 2D ortho setup
	int windowHeight;

    enum {
        CAM_MODE_NULL,
        CAM_MODE_FPS,
        CAM_MODE_LOOKAT,
		CAM_MODE_2D,
        CAM_MODE_LAST, //ALL MODES AFTER THIS WILL BE IGNORED
        CAM_MODE_CHASE,
        CAM_MODE_FLY
    } camModes;
    
	void SetMode(int MODE) {
		this->mode = MODE;
	}
	
	int GetMode() {
		return this->mode;
	}

    Camera(int MODE) {
		SetMode(MODE);
		pos.z = 10;
        resInf = new ResourceInf();
        resInf->Set("Camera", "", "",1);
    }
    
    Camera() : Camera(CAM_MODE_NULL) {}
    
    ~Camera() {
        delete resInf;
    }
    

    inline void MoveAhead(float d) {pos.z+=d;}
	inline void MoveBack(float d) { pos.z -= d; }
    inline void MoveRight(float d) {pos.x+=d;}
    inline void MoveLeft(float d) {pos.x-=d;}
    inline void MoveDown(float d) {pos.y-=d;}
    inline void MoveUp(float d) {pos.y+=d;}
    
    inline void RollRight(float d) {rot.z+=d;}
    inline void RollLeft(float d) {rot.z-=d;}
    inline void PitchUp(float d) {rot.x-=d;}
    inline void PitchDown(float d) {rot.x+=d;}
    inline void YawRight(float d) {rot.y+=d;}
    inline void YawLeft(float d) {rot.y-=d;}
    
    void ViewFromCurrent() {
        glRotatef(rot.x, 1., 0., 0.);
        glRotatef(rot.y, 0., 1., 0.);
        glRotatef(rot.z, 0., 0., 1.);
        glTranslatef(-pos.x, -pos.y, -pos.z);
    }
    
    void Update(float deltaT, GameObject* obj) {
   //     if (obj==this) return;
        if (mode==CAM_MODE_LOOKAT) {
            aluLookAt(pos.x, pos.y, pos.z, obj->pos.x, obj->pos.y, obj->pos.z, 0.,1.,0.);
        }

		if (mode == CAM_MODE_2D) {
		//	aluLookAt(0., 0., 0., obj->pos.x, obj->pos.y, obj->pos.z, 0., 1., 0.);
			//ViewFromCurrent();
		}
        
        if (mode==CAM_MODE_FPS || mode==CAM_MODE_FLY) {
            ViewFromCurrent();
        }
        
    }
    
    void NextMode() {
        mode++;
        if (mode>=CAM_MODE_LAST) {
            mode= CAM_MODE_NULL + 1;
        }
        if (mode==CAM_MODE_LOOKAT) printf("\tCameraMode:LOOKAT");
        if (mode==CAM_MODE_FPS) printf("\tCameraMode:FIRSTPERSON");
        if (mode==CAM_MODE_FLY) printf("\tCameraMode:FLY");
        if (mode==CAM_MODE_CHASE) printf("\tCameraMode:CHASE");
		if (mode == CAM_MODE_2D) printf("\tCameraMode:2D");
    }
    
	void processKeyDownWin(int key, float distStep) {
		if (mode == CAM_MODE_LOOKAT) {
			switch (key) {
			case AL_KEY_UP:
				MoveAhead(distStep);
				break;
			case AL_KEY_DOWN:
				MoveBack(distStep);
				break;
			case AL_KEY_LEFT:
				MoveLeft(distStep);
				break;
			case AL_KEY_RIGHT:
				MoveRight(distStep);
				break;
			case '3':
				MoveDown(distStep);
				break;
			case '9':
				MoveUp(distStep);
				break;
			default:
				break;
			}
		}
	}

    void processKeyDownMac(int key, float distStep) {
    
       if (mode==CAM_MODE_FPS) {
        switch (key) {
            case '8':
                MoveAhead(distStep);
                break;
            case '2':
                MoveBack(distStep);
                break;
            case '4':
                MoveLeft(distStep);
                break;
            case '6':
                MoveRight(distStep);
                break;
            case '3':
                MoveDown(distStep);
                break;
            case '9':
                MoveUp(distStep);
                break;
            default:
                break;
        }
       }
    }
};

#define C_float float
#define Sin sin
#define Cos cos


char* New_String(const char* sz) {return ((char*)sz);}


void alPushI (int cmd, int x, int y);
void alPushP (int cmd, void* param1, void* param2);
int alLoadModel(ResourceInf* r);
void alDrawModel (int id, bool wireframe = false);
/*
void alBillBoardBegin();
void alBillBoardEnd();
void alLoadIdentity();
void alPushMatrix();
void alPopMatrix();
void alaluLookAt(float x1, float y1, float z1, float x2, float y2, float z2, float x, float y, float z);
void alTriangle(float size);
void alLine(float x1, float y1, float x2, float y2);
void alCircle(float r, int segments);
void alRect(float w, float h);
void alLoadModel(char* alx, char* tga, int id, float size);
void alScaleModel(int id, float sx, float sy, float sz);
void alAlphaTest (int enable, float fA = 0.4); // 0=>unset ; 1=>set GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA
float alModelBounds(int id);
void alDrawModelTranslateRotate (int id, float posx = 0.0, float posy = 0.0, float posz = 0.0, float angle = 0.0, float x = 0.0, float y = 0.0, float z = 0.0, int rotatefirst = 0, int billboard = 0);
void alTranslateRotate (float posx = 0.0, float posy = 0.0, float posz = 0.0, float angle =0.0, float x = 0.0, float y = 0.0, float z = 0.0);
*/
#endif
