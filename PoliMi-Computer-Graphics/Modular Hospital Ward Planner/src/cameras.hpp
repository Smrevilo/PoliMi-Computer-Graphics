

void SetProjections(ModularHospitalWardPlanner *A) {
/**************
 Using A->SetMatrix(i, M) where:
 i -> index of the matrix to set (0 .. 7)
 M -> projection matrix to use

 define eight different types of projections.
 
 0 - Ortogonal Front (already given)
 1 - Isometric - z pointing bottom-left, and x pointing bottom-right
 2 - Dimetric, with an angle of 45 degree, z pointing bottom-right, and x pointing top-right
 3 - Trimetric, with an angle of alpha of 60 degree, and beta of 15 degrees, z pointing bottom-right, and x pointing top-right
 4 - Create a Cabinet projection, with the z axis at an angle of 40 degrees, pointing bottom-left
 5 - Create a perspective projection, with a Fov-y of  95 degrees
 6 - Create a perspective projection, with a Fov-y of  28 degrees (zoom)
 7 - Create a perspective projection, with a Fov-y of 150 degrees (wide)

 Projection of type 0 is already done for you. Please create the other seven cases.
 For all the projections, the aspect ratio is supposed to be 4:3 (assuming the application
 is used on a screen with square pixels). Near plane is at 0.1 for perspective and orthogonal, while it is at -500 for isometric, dimetric, trimenteric and cabinet. The far plane is always at 500.
 For the parlallel projections, the half-width of the screen in world coordinates is assumed to be 20.

 The directions of all the axes described in the comments above, please refer to the view as seen when the program starts: if you rotate the view, the directions will change and will no longer be as requested. Please also refrain from changing the size of the screen when saving the screen caputres for the delivery.

***************/
	glm::mat4 M;

	glm::mat4 vulkanCorrection = glm::mat4(
		1.0f,		0,			0,			0,
		0,			-1.0f,		0,			0,
		0,			0,			1.0f,		0,
		0,			0,			0,		1.0f);

	// Ortogonal Front
	// this is the only one correct, and that should not be modified
	M = glm::mat4(
		1.0f / 20.0f,0,			0,						0,
		0,			-4.0f / 60.f,0,						0,
		0,			0,			1.0f/(0.1f-500.0f),		0,
		0,			0,			0.1f/(0.1f-500.0f),		1);
	A->SetMatrix(0,  M);

	// Isometric - z pointing bottom-left, and x pointing bottom-right
	glm::mat4 RY2;
	glm::mat4 RX2;
	RY2 = glm::rotate(glm::mat4(1.0f), glm::radians(-45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	RX2 = glm::rotate(glm::mat4(1.0f), glm::radians(35.264f), glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 Ortho = glm::ortho(-20.0f, 20.0f, -15.0f, 15.0f, -500.0f, 500.0f);
	M = vulkanCorrection * Ortho * RX2 * RY2;
	A->SetMatrix(1,  M);

	// Dimetric, with an angle of 45 degree, z pointing bottom-right, and x pointing top-right
	glm::mat4 RY3 = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 RX3 = glm::rotate(glm::mat4(1.0f), glm::radians(-45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	M = vulkanCorrection * Ortho * RX3 * RY3;
	A->SetMatrix(2,  M);

	// Trimetric, with an angle of alpha of 60 degree, and beta of 15 degrees, z pointing bottom-right, and x pointing top-right
	glm::mat4 RY4 = glm::rotate(glm::mat4(1.0f), glm::radians(15.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 RX4 = glm::rotate(glm::mat4(1.0f), glm::radians(-60.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	M = vulkanCorrection * Ortho * RX4 * RY4;
	A->SetMatrix(3,  M);

	// Create a Cabinet projection, with the z axis at an angle of 40 degrees, pointing bottom-left
	glm::mat4 Shear = glm::mat4(
		1.0f,		0,			0,	0,
		0,			1.0f,		0,	0,
		-0.5f * glm::cos(glm::radians(40.0f)), -0.5f * glm::sin(glm::radians(40.0f)), 1.0f, 0,
		0,			0,			0,												1.0f);

	M = vulkanCorrection * Ortho * Shear;
	A->SetMatrix(4,  M);

	// Create a perspective projection, with a Fov-y of  95 degrees
	M = glm::perspective(glm::radians(95.0f), 4.0f / 3.0f, 0.1f, 500.0f);
	A->SetMatrix(5,  vulkanCorrection * M);

	// Create a perspective projection, with a Fov-y of  28 degrees (zoom)
	M = glm::perspective(glm::radians(28.0f), 4.0f / 3.0f, 0.1f, 500.0f);
	A->SetMatrix(6,  vulkanCorrection * M);

	// Create a perspective projection, with a Fov-y of 150 degrees (wide)
	M = glm::perspective(glm::radians(150.0f), 4.0f / 3.0f, 0.1f, 500.0f);
	A->SetMatrix(7,  vulkanCorrection * M);

}
