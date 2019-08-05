#define GLUT_DISABLE_ATEXIT_HACK
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECUreal_NO_WARNINGS
#include <stdio.h>
#include <gl/glut.h>
#include <math.h>
#define MAX_V 100000
#define MAX_F 100000
FILE *fp;
void calFaceNormal();
void calVertexNormal();
struct Vertex{
	float x, y, z; //x,y,z coordinates
};
struct Tri{
	long vertIDs[3]; //edge information
};
struct Normal
{
	float norm[3];
};
struct Mesh{
	Vertex verts[MAX_V];
	Tri faces[MAX_F];
	Normal Fnorms[MAX_F];
	Normal Tnorms[MAX_V];
};
long nVerts, nFaces;
// for gluLookAt parameters
float cameraPos[3]; //camera position
float cameraLookAt[3]; //camera Look at
float cameraUpvector[3] = { 0.0f, 1.0f, 0.0f }; // upvector
float lightPos[3];
float surfaceColor[3] = { 1, 1, 0 }; //gray
float g_x_rot, g_y_rot;
float z_near, z_far;
float dx, dy, dz; // for zooming
bool flag_f = true;
bool flag_n = false;
bool flag_l = false;
Mesh Data;
void init(void)
{
	int i, j;
	float xMin = 1000., xMax = -1000.;
	float yMin = 1000., yMax = -1000.;
	float zMin = 1000., zMax = -1000.;
	float xCenter, yCenter, zCenter;
	char wideAxis;
	fp = fopen("bunny1.txt", "r");
	fscanf(fp, "%d %d\n", &nVerts, &nFaces);
	for (i = 0; i<nVerts; i++)
		fscanf(fp, "%f %f %f", &Data.verts[i].x, &Data.verts[i].y, &Data.verts[i].z);
	for (i = 0; i<nFaces; i++)
		for (j = 0; j<3; j++)
			fscanf(fp, "%d", &Data.faces[i].vertIDs[j]);
	fclose(fp);
	calFaceNormal();
	calVertexNormal();
	// find lookat parameters
	// max and min coordinates
	for (i = 0; i<nVerts; i++) {
		if (xMin>Data.verts[i].x) xMin = Data.verts[i].x;
		if (yMin>Data.verts[i].y) yMin = Data.verts[i].y;
		if (zMin>Data.verts[i].z) zMin = Data.verts[i].z;
		if (xMax<Data.verts[i].x) xMax = Data.verts[i].x;
		if (yMax<Data.verts[i].y) yMax = Data.verts[i].y;
		if (zMax<Data.verts[i].z) zMax = Data.verts[i].z;
	}
	xCenter = (xMax + xMin) / 2.;
	yCenter = (yMax + yMin) / 2.;
	zCenter = (zMax + zMin) / 2.;
	for (int i = 0; i < nVerts; i++) {
		Data.verts[i].x -= xCenter;
		Data.verts[i].y -= yCenter;
		Data.verts[i].z -= zCenter;
	}
	// find the wide axis
	if ((fabs(xMax - xMin) >= fabs(yMax - yMin)) && (fabs(xMax - xMin) >= fabs(zMax - zMin))) wideAxis = 'X';
	else if ((fabs(yMax - yMin)>fabs(xMax - xMin)) && (fabs(yMax - yMin)>fabs(zMax - zMin))) wideAxis = 'Y';
	else if ((fabs(zMax - zMin)>fabs(yMax - yMin)) && (fabs(zMax - zMin)>fabs(xMax - xMin))) wideAxis = 'Z';
	// determine LookAt parameter
	if (wideAxis == 'X') {
		cameraPos[0] = 0; //camera position
		cameraPos[1] = 0;
		cameraPos[2] = zMax + fabs(xMax - xMin) - zCenter;
		lightPos[0] = cameraPos[0]; //light position
		lightPos[1] = cameraPos[1];
		lightPos[2] = cameraPos[2];
		cameraLookAt[0] = 0; //camera LookAt
		cameraLookAt[1] = 0;
		cameraLookAt[2] = 0;
		z_near = 0.5*fabs(xMax - xMin);
		z_far = 10.*z_near;
	}

	if (wideAxis == 'Y') {
		cameraPos[0] = 0; //camera position
		cameraPos[1] = 0;
		cameraPos[2] = zMax + fabs(yMax - yMin)-zCenter;
		lightPos[0] = cameraPos[0]; //light position
		lightPos[1] = cameraPos[1];
		lightPos[2] = cameraPos[2];
		cameraLookAt[0] = 0; //camera LookAt
		cameraLookAt[1] = 0;
		cameraLookAt[2] = 0;
		z_near = 0.5*fabs(yMax - yMin);
		z_far = 10.*z_near;
	}

	if (wideAxis == 'Z') {
		cameraPos[0] = xMax + fabs(zMax - zMin)-xCenter; //camera position
		cameraPos[1] = 0;
		cameraPos[2] = 0;
		lightPos[0] = cameraPos[0]; //light position
		lightPos[1] = cameraPos[1];
		lightPos[2] = cameraPos[2];
		cameraLookAt[0] = 0; //camera LookAt
		cameraLookAt[1] = 0;
		cameraLookAt[2] = 0;
		z_near = 0.5*fabs(zMax - zMin);
		z_far = 10.*z_near;
	}
	// Since the center is shifted to the origin..
	dx = (cameraPos[0]) / 20.;
	dy = (cameraPos[1]) / 20.;
	dz = (cameraPos[2]) / 20.;
}
//void calFaceNormal(long v1, long v2, long v3, float *norm)
//Data.faces[faceIDs].vertIDs[0] - 1, Data.faces[faceIDs].vertIDs[1] - 1, Data.faces[faceIDs].vertIDs[2] - 1
void calFaceNormal()
//important in shading
{
	float e1[3], e2[3];
	for (int i = 0; i < nFaces; i++) {
			e1[0] = Data.verts[Data.faces[i].vertIDs[1] - 1].x - Data.verts[Data.faces[i].vertIDs[0] - 1].x;
			e1[1] = Data.verts[Data.faces[i].vertIDs[1] - 1].y - Data.verts[Data.faces[i].vertIDs[0] - 1].y;
			e1[2] = Data.verts[Data.faces[i].vertIDs[1] - 1].z - Data.verts[Data.faces[i].vertIDs[0] - 1].z;
			e2[0] = Data.verts[Data.faces[i].vertIDs[2] - 1].x - Data.verts[Data.faces[i].vertIDs[0] - 1].x;
			e2[1] = Data.verts[Data.faces[i].vertIDs[2] - 1].y - Data.verts[Data.faces[i].vertIDs[0] - 1].y;
			e2[2] = Data.verts[Data.faces[i].vertIDs[2] - 1].z - Data.verts[Data.faces[i].vertIDs[0] - 1].z;
			// cross product e1 and e2
			Data.Fnorms[i].norm[0] = e1[1] * e2[2] - e1[2] * e2[1];
			Data.Fnorms[i].norm[1] = e1[2] * e2[0] - e1[0] * e2[2];
			Data.Fnorms[i].norm[2] = e1[0] * e2[1] - e1[1] * e2[0];
			//normalization
			float len = (float)sqrt(Data.Fnorms[i].norm[0] * Data.Fnorms[i].norm[0] + Data.Fnorms[i].norm[1] * Data.Fnorms[i].norm[1] + Data.Fnorms[i].norm[2] * Data.Fnorms[i].norm[2]);
			if (len == 0.) len = 1.;
			Data.Fnorms[i].norm[0] /= len;
			Data.Fnorms[i].norm[1] /= len;
			Data.Fnorms[i].norm[2] /= len;
	}
}
void calVertexNormal() {
	for (int i = 0; i < nFaces; i++)
		for (int j = 0; j < 3; j++)
			for (int k = 0; k < 3; k++)
				Data.Tnorms[Data.faces[i].vertIDs[j]].norm[k] += Data.Fnorms[i].norm[k];
	for (int i = 0; i < nVerts; i++) {
		float len = (float)sqrt(Data.Tnorms[i].norm[0] * Data.Tnorms[i].norm[0] + Data.Tnorms[i].norm[1] * Data.Tnorms[i].norm[1] + Data.Tnorms[i].norm[2] * Data.Tnorms[i].norm[2]);
		if (len == 0.) len = 1.;
		Data.Tnorms[i].norm[0] /= len;
		Data.Tnorms[i].norm[1] /= len;
		Data.Tnorms[i].norm[2] /= len;
	}
}
void Display(void)
{
	glClearColor(1.0, 1.0, 1.0, 0.0); // set background color -> blue
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//clear and paint background
	glColor3f(surfaceColor[0], surfaceColor[1], surfaceColor[2]); //set surface color
	//model view matrix setting
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	//view matrix setting
	gluLookAt(cameraPos[0], cameraPos[1], cameraPos[2], //camera position
		cameraLookAt[0], cameraLookAt[1], cameraLookAt[2], //camera look at
		cameraUpvector[0], cameraUpvector[1], cameraUpvector[2]); //camera upvector
	glRotatef(g_x_rot, 1.0f, 0.0f, 0.0f); //controlled by key
	glRotatef(g_y_rot, 0.0f, 1.0f, 0.0f);
	if (flag_f == true) {
		glBegin(GL_TRIANGLES);
		for (long faceIDs = 0; faceIDs < nFaces; faceIDs++) {
			glNormal3f(Data.Fnorms[faceIDs].norm[0], Data.Fnorms[faceIDs].norm[1], Data.Fnorms[faceIDs].norm[2]);
			for (int p = 0; p < 3; p++)
				glVertex3f(Data.verts[Data.faces[faceIDs].vertIDs[p] - 1].x,
					Data.verts[Data.faces[faceIDs].vertIDs[p] - 1].y,
					Data.verts[Data.faces[faceIDs].vertIDs[p] - 1].z);
		}
		glEnd();
	}
	else if (flag_n == true) {
		glBegin(GL_TRIANGLES);
		for (long faceIDs = 0; faceIDs < nFaces; faceIDs++) {
			for (int p = 0; p < 3; p++) {
				glNormal3f(Data.Tnorms[Data.faces[faceIDs].vertIDs[p]].norm[0], Data.Tnorms[Data.faces[faceIDs].vertIDs[p]].norm[1], Data.Tnorms[Data.faces[faceIDs].vertIDs[p]].norm[2]);
				glVertex3f(Data.verts[Data.faces[faceIDs].vertIDs[p] - 1].x,
							Data.verts[Data.faces[faceIDs].vertIDs[p] - 1].y,
							Data.verts[Data.faces[faceIDs].vertIDs[p] - 1].z);
			}
		}
		glEnd();
	}
	else if (flag_l == true) {
		glColor3f(0.3, 0.3, 0.3);
		for (long faceIDs = 0; faceIDs < nFaces; faceIDs++) {
			glBegin(GL_LINE_LOOP);
			glNormal3f(Data.Fnorms[faceIDs].norm[0], Data.Fnorms[faceIDs].norm[1], Data.Fnorms[faceIDs].norm[2]);
			for (int p = 0; p < 3; p++) {
				glVertex3f(Data.verts[Data.faces[faceIDs].vertIDs[p] - 1].x,
					Data.verts[Data.faces[faceIDs].vertIDs[p] - 1].y,
					Data.verts[Data.faces[faceIDs].vertIDs[p] - 1].z);
			}
			glEnd();
		}
	}
	glFlush();
}

void Reshape(int w, int h)
{
	float fov_y, aspect;

	if (h == 0) h = 1.;
	aspect = (float)w / (float)h;
	fov_y = 60.0f;
	glViewport(0, 0, w, h);
	//projection matrix setting 
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//mode of projection
	//glOrtho(-1.0, 1.0, -1.0, 1.0, 0.0, 100.0); //Orthogonal projection
	gluPerspective(fov_y, aspect, z_near, z_far);

}

void Key(int key, int x, int y)
{
	if (key == GLUT_KEY_UP)
		g_x_rot -= 5.0f;
	if (key == GLUT_KEY_DOWN)
		g_x_rot += 5.0f;
	if (key == GLUT_KEY_LEFT)
		g_y_rot -= 5.0f;
	if (key == GLUT_KEY_RIGHT)
		g_y_rot += 5.0f;
	if (key>365.0f)
		g_x_rot = 0.0f;
	if (key<0.0f)
		g_x_rot = 365.0f;
	if (key>365.0f)
		g_y_rot = 0.0f;
	if (key<0.0f)
		g_y_rot = 365.0f;
	// Zoom key
	if (key == GLUT_KEY_F1){
		cameraPos[0] -= dx;
		cameraPos[1] -= dy;
		cameraPos[2] -= dz;
	}
	if (key == GLUT_KEY_F2){
		cameraPos[0] += dx;
		cameraPos[1] += dy;
		cameraPos[2] += dz;
	}
	if (key == GLUT_KEY_F3) {
		if (flag_f == true)
			flag_f = false;
		else {
			flag_f = true;
			flag_n = false;
		}
	}
	if (key == GLUT_KEY_F4) {
		if (flag_n == true)
			flag_n = false;
		else {
			flag_f = false;
			flag_n = true;
		}
	}
	if (key == GLUT_KEY_F5) {
		if (flag_l == true)
			flag_l = false;
		else {
			flag_l = true;
		}
	}
	//refresh window
	glutPostRedisplay();
}

void SetupRender(void)
{
	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST); // z-buffer check, front or back test(should be included in Line+Face..)
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);
	// setting material
	GLfloat mat_ambient[] = { 0.1f, 0.1f, 0.1f, 1.0f };
	GLfloat mat_diffuse[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	GLfloat mat_specular[] = { 0.1f, 0.1f, 0.1f, 1.0f };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
	glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, 40);
	glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
	// setting light source
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
	glEnable(GL_LIGHT0);
}
void main(void)
{
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_SINGLE);
	glutCreateWindow("OpenGl Example");
	init();
	SetupRender();
	glutReshapeFunc(Reshape);
	glutSpecialFunc(Key);
	glutDisplayFunc(Display);
	glutMainLoop();
}