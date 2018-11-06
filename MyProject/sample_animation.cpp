#include <GL/gl.h>
#include <GL/glut.h>

GLfloat top = -0.9;
void disp( void ) {
	glClear(GL_COLOR_BUFFER_BIT);

	glBegin(GL_POLYGON);
		glVertex2f(-0.9 , -0.9);
		glVertex2f(0 , top);
		glVertex2f(0.9 , -0.9);
	glEnd();

	glFlush();
}

void timer(int value) {
	static GLboolean isUp = GL_TRUE;

	if (top > 0.9F) isUp = GL_FALSE;
	else if (top <= -0.9F) isUp = GL_TRUE;
	top += (isUp == GL_TRUE ? 0.01 : -0.01);

	glutPostRedisplay();
	glutTimerFunc(50 , timer , 0);
}

int main(int argc , char ** argv) {
	glutInit(&argc , argv);
	glutInitWindowPosition(100 , 50);
	glutInitWindowSize(400 , 300);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);

	glutCreateWindow("Kitty on your lap");
	glutDisplayFunc(disp);
	glutTimerFunc(100 , timer , 0);

	glutMainLoop();
	return 0;
}