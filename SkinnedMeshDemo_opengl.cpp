//  A simplest Skinned Mesh demo, written by n5, 2008.10,

//  My email:happyfirecn@yahoo.com.cn

//  My blog: http://blog.csdn.net/n5

 

#include <GL/glut.h>

 

#define NULL 0

 

//-------------------------------------------------------------

 

class BoneOffset

{

public:

    //BoneOffset transform a vertex from mesh space to bone space.

    //In other words, it is the offset from mesh space to a bone's space.

    //For each bone, there is a BoneOffest.

    //If we add the offset to the vertex's pos (in mesh space), we get the vertex's pos in bone space

    //For example: if a vertex's pos in mesh space is (100,0,0), the bone offset is (-20,0,0), so the vertex's pos in bone space is (80,0,0)

    //Actually, BoneOffset is the invert transform of that we place a bone in mesh space, that is (-20,0,0) means the bone is at (20,0,0) in mesh space

    float m_offx, m_offy, m_offz;

};

 

//----------------------------------------------------------------

 

class Bone

{

public:

    Bone() {}

    Bone(float x, float y, float z):m_pSibling(NULL),m_pFirstChild(NULL),m_pFather(NULL),m_x(x),m_y(y),m_z(z){}

 

    ~Bone() {}

 

    Bone* m_pSibling;

    Bone* m_pFirstChild;

    Bone* m_pFather; //only for draw bone

 

    void SetFirstChild(Bone* pChild) { m_pFirstChild = pChild; m_pFirstChild->m_pFather = this; }

    void SetSibling(Bone* pSibling) { m_pSibling = pSibling; m_pSibling->m_pFather = m_pFather; }

 

    float m_x, m_y, m_z;//pos in its parent's space 

 

    float m_wx, m_wy, m_wz; //pos in world space

 

    //give father's world pos, compute the bone's world pos

    void ComputeWorldPos(float fatherWX, float fatherWY, float fatherWZ)

    {

       m_wx = fatherWX+m_x;

       m_wy = fatherWY+m_y;

       m_wz = fatherWZ+m_z;

 

       if(m_pSibling!=NULL)

           m_pSibling->ComputeWorldPos(fatherWX, fatherWY, fatherWZ);

 

       if(m_pFirstChild!=NULL)

           m_pFirstChild->ComputeWorldPos(m_wx, m_wy, m_wz);

    }

 

    BoneOffset m_boneOffset;

   

    //called after compute world pos when bone loaded but not animated

    void ComputeBoneOffset()

    {

       m_boneOffset.m_offx = -m_wx;

       m_boneOffset.m_offy = -m_wy;

       m_boneOffset.m_offz = -m_wz;

 

       if(m_pSibling!=NULL)

           m_pSibling->ComputeBoneOffset();

       if(m_pFirstChild!=NULL)

           m_pFirstChild->ComputeBoneOffset();

    }

 

    void Draw()

    {

       glColor3f(0,0,1.0);

       glPointSize(4);

       glBegin(GL_POINTS);     

       glVertex3f(m_wx,m_wy,m_wz);

       glEnd();

       if(m_pFather!=NULL)

       {

           glBegin(GL_LINES);                       

              glVertex3f(m_pFather->m_wx,m_pFather->m_wy,m_pFather->m_wz);

              glVertex3f(m_wx,m_wy,m_wz);

           glEnd();

       }

 

       if(m_pSibling!=NULL)

           m_pSibling->Draw();

       if(m_pFirstChild!=NULL)

           m_pFirstChild->Draw();

      

    }

};

 

//--------------------------------------------------------------

 

#define MAX_BONE_PER_VERTEX 4

 

 

class Vertex

{

public:

    Vertex():m_boneNum(0)

    {

    }

 

    void ComputeWorldPosByBone(Bone* pBone, float& outX, float& outY, float& outZ)

    {

       //step1: transform vertex from mesh space to bone space

       outX = m_x+pBone->m_boneOffset.m_offx;

       outY = m_y+pBone->m_boneOffset.m_offy;

       outZ = m_z+pBone->m_boneOffset.m_offz;

 

       //step2: transform vertex from bone space to world sapce

       outX += pBone->m_wx;

       outY += pBone->m_wy;

       outZ += pBone->m_wz;

    }

 

    void BlendVertex()

    {//do the vertex blending,get the vertex's pos in world space

 

       m_wX = 0;

       m_wY = 0;

       m_wZ = 0;

 

       for(int i=0; i<m_boneNum; ++i)

       {

           float tx, ty, tz;

           ComputeWorldPosByBone(m_bones[i], tx, ty, tz);

           tx*= m_boneWeights[i];

           ty*= m_boneWeights[i];

           tz*= m_boneWeights[i];

 

           m_wX += tx;

           m_wY += ty;

           m_wZ += tz;

       }

    }

 

    float m_x, m_y, m_z; //local pos in mesh space

 

    float m_wX, m_wY, m_wZ;//blended vertex pos, in world space

 

    //skin info

    int m_boneNum;

    Bone* m_bones[MAX_BONE_PER_VERTEX];

    float m_boneWeights[MAX_BONE_PER_VERTEX];

 

    void SetBoneAndWeight(int index, Bone* pBone, float weight)

    {

       m_bones[index] = pBone;

       m_boneWeights[index] = weight;    

    }

};

 

//-----------------------------------------------------------

class SkinMesh

{

public:

    SkinMesh():m_vertexNum(0){}

 

    SkinMesh(int vertexNum):m_vertexNum(vertexNum)

    {     

       m_vertexs = new Vertex[vertexNum];

    }

 

    ~SkinMesh()

    {

       if(m_vertexNum>0)

           delete[] m_vertexs;

    }  

 

    void UpdateVertices()

    {

       for(int i=0; i<m_vertexNum; ++i)

       {

           m_vertexs[i].BlendVertex();

       }

    }

 

    void DrawStaticMesh(float x, float y, float z)

    {

       glColor3f(0,1.0,0);

       glPointSize(4);

       glBegin(GL_POINTS);

       for(int i=0; i<m_vertexNum; ++i)

           glVertex3f(m_vertexs[i].m_x+x,m_vertexs[i].m_y+y,m_vertexs[i].m_z+z);

       glEnd();

      

       glBegin(GL_LINE_LOOP);

       for(int i=0; i<m_vertexNum; ++i)

           glVertex3f(m_vertexs[i].m_x+x,m_vertexs[i].m_y+y,m_vertexs[i].m_z+z);

       glEnd();

    }

 

    void Draw()

    {

       glColor3f(1.0,0, 0);

       glPointSize(4);

       glBegin(GL_POINTS);

       for(int i=0; i<m_vertexNum; ++i)

           glVertex3f(m_vertexs[i].m_wX,m_vertexs[i].m_wY,m_vertexs[i].m_wZ);

       glEnd();

      

       glBegin(GL_LINE_LOOP);

       for(int i=0; i<m_vertexNum; ++i)

           glVertex3f(m_vertexs[i].m_wX,m_vertexs[i].m_wY,m_vertexs[i].m_wZ);

       glEnd();

    }

 

 

    int m_vertexNum;

    Vertex* m_vertexs; //array of vertices in mesh  

 

};

 

//--------------------------------------------------------------

 

Bone* g_boneRoot;

Bone* g_bone1, *g_bone2, *g_bone31, *g_bone32;

 

void buildBones()

{

    g_boneRoot = new Bone(0, 0, 0);

   

    g_bone1 = new Bone(0.2, 0, 0);

 

    g_bone2 = new Bone(0.2, 0, 0);

 

    g_bone31 = new Bone(0.2, 0.1, 0);

    g_bone32 = new Bone(0.2, -0.1, 0);

 

    g_boneRoot->SetFirstChild(g_bone1);

    g_bone1->SetFirstChild(g_bone2);

    g_bone2->SetFirstChild(g_bone31);

    g_bone31->SetSibling(g_bone32);

}

 

void deleteBones()

{

    delete g_boneRoot;

    delete g_bone1;

    delete g_bone2;

    delete g_bone31;

    delete g_bone32;

}

 

void animateBones()

{

    static int dir=-1, dir2=-1;

    //animate bones manually

 

    g_bone1->m_y +=0.00001f*dir;   

 

    if(g_bone1->m_y<-0.2 || g_bone1->m_y>0.2)

       dir*=-1;

   

    g_bone32->m_x +=0.00001f*dir2;

 

    if(g_bone32->m_x<0 || g_bone32->m_x>0.2)

       dir2*=-1;

}

 

SkinMesh* g_mesh;

 

 

 

void buildMesh()

{

    float _meshData[]=

    {//x,y,z

       -0.1,0.05,0,            

       0.1,0.05,0,             

       0.3,0.05,0,      

       0.45,0.06,0,

       0.6,0.15,0,

       0.65,0.1,0,

      

       0.5,0,0,

 

       0.65,-0.1,0,

       0.6,-0.15,0,

       0.45,-0.06,0,

       0.3,-0.05,0,     

       0.1,-0.05,0,

       -0.1,-0.05,0,    

    };

 

    float _skinInfo[]=

    {//bone_num,bone id(0,1,2,31 or 32), bone weight 1~4,

       1,  0, -1, -1, -1,    1.0, 0.0, 0.0, 0.0,

       2,  0,  1, -1, -1, 0.5, 0.5, 0.0, 0.0,

       2,  1,  2, -1, -1,  0.5, 0.5, 0.0, 0.0,

       2,  2,  31, -1, -1, 0.3, 0.7, 0.0, 0.0,

       2,  2,  31, -1, -1, 0.2, 0.8, 0.0, 0.0,

       1,  31, -1, -1, -1, 1.0, 0.0, 0.0, 0.0,

 

       2,  31, 32, -1, -1, 0.5, 0.5, 0.0, 0.0,

 

       1,  32, -1, -1, -1, 1.0, 0.0, 0.0, 0.0,

       2,  2,  32, -1, -1, 0.2, 0.8, 0.0, 0.0,

       2,  2,  32, -1, -1, 0.3, 0.7, 0.0, 0.0,

       2,  1,  2, -1, -1,  0.5, 0.5, 0.0, 0.0,

       2,  0,  1, -1, -1, 0.5, 0.5, 0.0, 0.0,

       1,  0, -1, -1, -1,    1.0, 0.0, 0.0, 0.0,

    };

   

    int vertexNum = sizeof(_meshData)/(sizeof(float)*3);

    g_mesh = new SkinMesh(vertexNum); 

    for(int i=0; i<vertexNum; ++i)

    {

       g_mesh->m_vertexs[i].m_x = _meshData[i*3];

       g_mesh->m_vertexs[i].m_y = _meshData[i*3+1];

       g_mesh->m_vertexs[i].m_z = _meshData[i*3+2];           

    }

 

    //set skin info

    for(int i=0; i<vertexNum; ++i)

    {

       g_mesh->m_vertexs[i].m_boneNum = _skinInfo[i*9];

       for(int j=0; j<g_mesh->m_vertexs[i].m_boneNum; ++j)

       {

           Bone* pBone = g_boneRoot;

           if(_skinInfo[i*9+1+j]==1)

              pBone = g_bone1;

           else if(_skinInfo[i*9+1+j]==2)

              pBone = g_bone2;

           else if(_skinInfo[i*9+1+j]==31)

              pBone = g_bone31;

           else if(_skinInfo[i*9+1+j]==32)

              pBone = g_bone32;

 

           g_mesh->m_vertexs[i].SetBoneAndWeight(j, pBone, _skinInfo[i*9+5+j]); 

       }

    }  

 

    //compute bone offset

    g_boneRoot->ComputeWorldPos(0, 0, 0);

    g_boneRoot->ComputeBoneOffset();

}

 

void deleteMesh()

{

    delete g_mesh;

}

 

void myInit()

{

    buildBones();

    buildMesh();

}

 

void myQuit()

{

    deleteBones();

    deleteMesh();

}

 

void myReshape(int width, int height)

{

    GLfloat h = (GLfloat) height / (GLfloat) width;

   

    glViewport(0, 0, (GLint) width, (GLint) height);

    glMatrixMode(GL_PROJECTION);

    glLoadIdentity();

//  glFrustum(-1.0, 1.0, -h, h, 5.0, 60.0);

    glFrustum(-1.0, 1.0, -h, h, 1.0, 100.0);

    glMatrixMode(GL_MODELVIEW);

    glLoadIdentity();

    glTranslatef(0.0, 0.0, -1.0);

}

 

void myDisplay(void)

{

    glClear(GL_COLOR_BUFFER_BIT);

 

    //draw original mesh

    g_mesh->DrawStaticMesh(0,0,0);

      

    //move bones

    animateBones();

 

    //update all bone's pos in bone tree

    g_boneRoot->ComputeWorldPos(0, 0, 0);

 

    //update vertex pos by bones, using vertex blending

    g_mesh->UpdateVertices();  

 

    //draw deformed mesh

    g_mesh->Draw();

 

    //draw bone

    g_boneRoot->Draw();

 

    glFlush();

    glutSwapBuffers();  

}

 

void myIdle(void)

{

    myDisplay();

}

 

int main(int argc, char *argv[])

{

    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);

    glutInitWindowPosition(100, 100);

    glutInitWindowSize(640, 480);

    glutCreateWindow("A simplest skinned mesh DEMO, by happyfirecn@yahoo.com.cn");

   

    glutDisplayFunc(myDisplay);

    glutReshapeFunc(myReshape);

    glutIdleFunc(myIdle);

   

    myInit();

    glutMainLoop();

    myQuit();

 

    return 0;

}
