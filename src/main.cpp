// This has been adapted from the Vulkan tutorial

// TO MOVE
#define JSON_DIAGNOSTICS 1
#include "modules/Starter.hpp"
#include "modules/IconMaker.hpp"
#include <cmath>
#include <glm/gtc/constants.hpp>
#include <unordered_map>
#include <unordered_set>

// The uniform buffer object used in this example
struct UniformBufferObject {
	alignas(16) glm::mat4 mvpMat;
	alignas(16) glm::mat4 mMat;
	alignas(16) glm::mat4 nMat;
	alignas(4) float specularStrength;
};

struct GlobalUniformBufferObject {
	alignas(16) glm::vec3 lightDir;
	alignas(16) glm::vec4 lightColor;
	alignas(16) glm::vec3 eyePos;
};



// The vertices data structures
// Example
struct Vertex {
	glm::vec3 pos;
	glm::vec2 UV;
	glm::vec3 norm;
};

#include "modules/Scene.hpp"

// Hash function for grid coordinates
struct GridCoordHash {
	size_t operator()(const std::tuple<int,int,int>& p) const noexcept {
		return std::hash<int>()(std::get<0>(p)) ^ (std::hash<int>()(std::get<1>(p)) << 1) ^ (std::hash<int>()(std::get<2>(p)) << 2);
	}
};


// MAIN !
class ModularHospitalWardPlanner : public BaseProject {
	protected:

	// Descriptor Layouts ["classes" of what will be passed to the shaders]
	DescriptorSetLayout DSL;

	// Vertex formats
	VertexDescriptor VD;

	// Pipelines [Shader couples]
	Pipeline P;
	Pipeline P_Transparent;
	
	// Descriptor sets
	DescriptorSet DS1;

	Scene SC;
	IconMaker IR;

	// Other application parameters
	float Ar;	//aspect ratio
	glm::vec3 Pos;			//current object pos
	glm::vec3 InitialScale;	//initial object scale
	float objectRotation = 0.0f;

	std::unordered_map<std::tuple<int,int,int>, std::string, GridCoordHash> placedObjects; //placed objects map
	int spawnCounter = 0;	//used for iding objects
	std::vector<std::string> objIds;
	int selectedObj = 0;
	std::unordered_map<std::string, float> objectScale;

	// First person view mode
	bool firstPersonMode = false;
	glm::vec3 fpCameraPos = glm::vec3(0.0f, 5.0f, 5.0f);
	glm::vec3 fpCameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 fpCameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	float fpCameraYaw = -90.0f;
	float fpCameraPitch = 0.0f;
	float fpCameraSpeed = 3.0f;
	// Mouse movement for FPV
	double lastMouseX = 0.0, lastMouseY = 0.0;
	double mouseDeltaX = 0.0, mouseDeltaY = 0.0;
	bool mouseInitialized = false;
// Helper to check key state
bool isKeyPressed(int key) {
	return glfwGetKey(window, key) == GLFW_PRESS;
}

void savePlacedObjects(const std::string& filename = "placed_objects.txt") {
	std::ofstream file(filename);
	if (!file.is_open()) {
		std::cerr << "Failed to open file for saving placed objects: " << filename << "\n";
		return;
	}
	for (const auto& [coord, id] : placedObjects) {
		if(id.find("_spawn_") == std::string::npos) continue;
		glm::mat4 objTransform = SC.I[SC.InstanceIds[id]].Wm;
		file << id << " ";
		for(int i = 0; i < 4; i++){
			for(int j = 0; j < 4; j++){

				file << ((objTransform[i][j] < 0.001 && objTransform[i][j] > -0.001) ? 0.0f : objTransform[i][j]) << " ";
			}
		}
		file << "\n";
	}

}
void loadPlacedObjects(const std::string& filename = "placed_objects.txt") {
	
	std::vector<std::string> toRemove;
	toRemove.reserve(SC.InstanceCount);
	for (int i = 0; i < SC.InstanceCount; ++i) {
		if (SC.I[i].id && SC.I[i].id->find("_spawn_") != std::string::npos) {
			toRemove.push_back(*SC.I[i].id);
		}
	}
	if (!toRemove.empty()) {
		vkDeviceWaitIdle(device);
		for (const auto& rid : toRemove) {
			SC.removeInstance(rid);
		}
	}


	placedObjects.clear();
	
	std::ifstream file(filename);
	std::string line;
	while (std::getline(file, line)) {
		std::istringstream iss(line);
		std::string id;
		glm::mat4 transform;
		if(!(iss >> id)) continue;
		for(int i = 0; i < 4; i++){
			for(int j = 0; j < 4; j++){
				iss >> transform[i][j];
			}
		}
		std::string baseId = id.substr(0, id.find("_spawn_"));
		std::string newId = baseId + "_spawn_" + std::to_string(spawnCounter++);
		SC.addInstance(newId, SC.MeshIds[baseId], SC.TextureIds[baseId], transform, DSL);
		placedObjects[std::make_tuple(transform[3][0], transform[3][1], transform[3][2])] = newId;
	}
	vkDeviceWaitIdle(device);
	vkFreeCommandBuffers(device, commandPool,
						static_cast<uint32_t>(commandBuffers.size()),
						commandBuffers.data());
	createCommandBuffers();
}
// Mouse callback for GLFW
static void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
	ModularHospitalWardPlanner* app = reinterpret_cast<ModularHospitalWardPlanner*>(glfwGetWindowUserPointer(window));
	if (!app->mouseInitialized) {
		app->lastMouseX = xpos;
		app->lastMouseY = ypos;
		app->mouseInitialized = true;
	}
	app->mouseDeltaX = xpos - app->lastMouseX;
	app->mouseDeltaY = ypos - app->lastMouseY;
	app->lastMouseX = xpos;
	app->lastMouseY = ypos;
}

	// Here you set the main application parameters
	void setWindowParameters() {
		// window size, titile and initial background
		windowWidth = 800;
		windowHeight = 600;
        windowTitle = "Modular Hospital Ward Planner";
    	windowResizable = GLFW_TRUE;
		initialBackgroundColor = {0.3f, 0.90f, 1.0f, 1.0f};

		// Descriptor pool sizes
		// allow for many dynamically spawned objects
		uniformBlocksInPool = 1000;
		texturesInPool = 1000;
		setsInPool = 1000;

		Ar = 4.0f / 3.0f;
	}

	// What to do when the window changes size
	void onWindowResize(int w, int h) {
		std::cout << "Window resized to: " << w << " x " << h << "\n";
		Ar = (float)w / (float)h;
	}

	// Here you load and setup all your Vulkan Models and Texutures.
	// Here you also create your Descriptor set layouts and load the shaders for the pipelines
	void localInit() {
		// Descriptor Layouts [what will be passed to the shaders]
		DSL.init(this, {
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS},
					{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT},
					{2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS}
				});

		// Vertex descriptors
		VD.init(this, {
				  {0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX}
				}, {
				  {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos),
				         sizeof(glm::vec3), POSITION},
				  {0, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, UV),
				         sizeof(glm::vec2), UV},
				  {0, 2, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, norm),
				         sizeof(glm::vec3), NORMAL}
				});

		// Pipelines [Shader couples]
		P.init(this, &VD, "shaders/PhongVert.spv", "shaders/PhongFrag.spv", {&DSL});
		P.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL,
 								    VK_CULL_MODE_NONE, false);
		
		P_Transparent.init(this, &VD, "shaders/PhongVert.spv", "shaders/TransparentFrag.spv", {&DSL});
		P_Transparent.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL,
 								    VK_CULL_MODE_BACK_BIT, true);  // back face culling + true enables blending for transparency

		// Load Scene
		SC.init(this, &VD, DSL, P, "assets/models/scene.json");

		// Init local variables
        Pos = glm::vec3(SC.I[SC.InstanceIds["obj"]].Wm[3]);
        objIds = SC.ModelIds;
		objIds.erase(std::remove(objIds.begin(), objIds.end(), "top"), objIds.end());
        objectScale = SC.ModelScales;
        if(!objIds.empty()) {
                InitialScale = glm::vec3(objectScale[objIds[selectedObj]]);
        }

		IR.init(this,
				 {{"potted1", "assets/Icons/M_PottedPlant_01.png"},
				  {"potted2", "assets/Icons/M_PottedPlant_02.png"},
				  {"aircondition", "assets/Icons/M_Aircondition_01.png"},
				  {"bed", "assets/Icons/M_Bed_01.png"},
				  {"bulletinboard", "assets/Icons/M_BulletinBoard_01.png"},
				  {"cabinet", "assets/Icons/M_Cabinet_01.png"},
				  {"closestool", "assets/Icons/M_Closestool_01.png"},
				  {"curtain", "assets/Icons/M_Curtain_01.png"},
				  {"door1", "assets/Icons/M_Door_01.png"},
				  {"door2", "assets/Icons/M_Door_02.png"},
				  {"door3", "assets/Icons/M_Door_03.png"},
				  {"nursesstation", "assets/Icons/M_NursesStation_01.png"},
				  {"pc", "assets/Icons/M_PC_01.png"},
				  {"poster", "assets/Icons/M_Poster_01.png"},
				  {"shelf", "assets/Icons/M_Shelf_01.png"},
				  {"socket", "assets/Icons/M_Socket_01.png"},
				  {"sofa", "assets/Icons/M_Sofa_01.png"},
				  {"tv", "assets/Icons/M_TV_01.png"},
				  {"trashcan", "assets/Icons/M_TrashCan_01.png"},
				  {"wardrobe", "assets/Icons/M_Wardrobe_01.png"},
				  {"window", "assets/Icons/M_Window_01.png"}},
				 windowWidth, windowHeight);
		// Set mouse callback
		glfwSetWindowUserPointer(window, this);
		glfwSetCursorPosCallback(window, mouseCallback);
	}

	// Here you create your pipelines and Descriptor Sets!
	void pipelinesAndDescriptorSetsInit() {
		// Create both pipelines
		P.create();
		P_Transparent.create();

		DS1.init(this, &DSL, {
						{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
						{1, TEXTURE, 0, SC.T[SC.TextureIds["t0"]]},
						{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
				});

		// Here you define the data set
		SC.pipelinesAndDescriptorSetsInit(DSL);
		IR.pipelinesAndDescriptorSetsInit();

		// Update the preview instance now that descriptor sets are ready
		SC.updateInstance("obj", SC.MeshIds[objIds[selectedObj]], SC.TextureIds[objIds[selectedObj]], DSL);
	}

	// Here you destroy your pipelines and Descriptor Sets!
	// All the object classes defined in Starter.hpp have a method .cleanup() for this purpose
	void pipelinesAndDescriptorSetsCleanup() {
		// Cleanup both pipelines
		P.cleanup();
		P_Transparent.cleanup();
		DS1.cleanup();

		SC.pipelinesAndDescriptorSetsCleanup();
		IR.pipelinesAndDescriptorSetsCleanup();
	}

	// Here you destroy all the Models, Texture and Desc. Set Layouts you created!
	// All the object classes defined in Starter.hpp have a method .cleanup() for this purpose
	// You also have to destroy the pipelines: since they need to be rebuilt, they have two different
	// methods: .cleanup() recreates them, while .destroy() delete them completely
    void localCleanup() {
       P.destroy();
	    P_Transparent.destroy();

		DSL.cleanup();

		SC.localCleanup();
		IR.localCleanup();
	}

	// Here it is the creation of the command buffer:
	// You send to the GPU all the objects you want to draw,
	// with their buffers and textures

	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
		P.bind(commandBuffer);
		DS1.bind(commandBuffer, P, 0, currentImage);
		// Ensure the preview object (id == "obj") is always transparent

		// draw not transparent objects
		for(int i = 0; i < SC.InstanceCount; i++) {
			std::string instanceId = *SC.I[i].id;
			if(instanceId == "obj") continue;
			P.bind(commandBuffer);
			SC.M[SC.I[i].Mid]->bind(commandBuffer);
			SC.DS[i]->bind(commandBuffer, P, 0, currentImage);
			vkCmdDrawIndexed(commandBuffer,
				static_cast<uint32_t>(SC.M[SC.I[i].Mid]->indices.size()), 1, 0, 0, 0);
		}

		// draw transparent object
		int objIdx = SC.InstanceIds["obj"];
		P_Transparent.bind(commandBuffer);
		SC.M[SC.I[objIdx].Mid]->bind(commandBuffer);
		SC.DS[objIdx]->bind(commandBuffer, P_Transparent, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
					static_cast<uint32_t>(SC.M[SC.I[objIdx].Mid]->indices.size()), 1, 0, 0, 0);

		// Draw UI icons
		IR.populateCommandBuffer(commandBuffer, currentImage, objIds[selectedObj]);
	}

	// Here is where you update the uniforms.
	// Very likely this will be where you will be writing the logic of your application.
	void updateUniformBuffer(uint32_t currentImage) {
		static bool debounce = false;
		static int curDebounce = 0;
		static bool invertedView = false;


		float deltaT;
		glm::vec3 m = glm::vec3(0.0f), r = glm::vec3(0.0f);
		bool fire = false;
		getSixAxis(deltaT, m, r, fire);
		if(invertedView){
			m.x = -m.x;
			m.z = -m.z;
		}

		int objectId = SC.InstanceIds["obj"];

		// First person mode input and camera update
		constexpr float GRID_SIZE = 1.0f;
		if (!firstPersonMode){
			static float movementTimer = 0.0f;
			static glm::vec3 prevMoveDir = glm::vec3(0.0f); //previous movement direction
			const float MOVEMENT_DIFF = 0.05f; //time between movements
			movementTimer -= deltaT;
			if(movementTimer < 0.0f) movementTimer = 0.0f;

			//move every MOVEMENT_DIFF when holding
			if(glm::length(m) > 0.0f) {
				//movement time elapsed or direction changed
				if(movementTimer <= 0.0f || m != prevMoveDir) {
						//if still pressing
						if(glm::length(m) > 0.0f) {
						//reset timer
						movementTimer = MOVEMENT_DIFF;
						//change pos
						glm::vec3 newPos = Pos + m * GRID_SIZE;
						newPos = glm::vec3(
							GRID_SIZE * std::round(newPos.x / GRID_SIZE),
							std::max(0.0f, GRID_SIZE * std::round(newPos.y / GRID_SIZE)),
							GRID_SIZE * std::round(newPos.z / GRID_SIZE)
						);
						Pos = newPos;
					}
				}
			}
			prevMoveDir = m;

			// Update object rotation
			float objectRot = glm::half_pi<float>() * std::round(objectRotation / glm::half_pi<float>());
			SC.I[objectId].Wm = glm::translate(glm::mat4(1), Pos) * glm::rotate(glm::mat4(1), objectRotation, glm::vec3(0,1,0)) * glm::scale(glm::mat4(1), InitialScale);
		} 
		else {
			// WASD movement
			glm::vec3 moveDir(0.0f);
			if (isKeyPressed(GLFW_KEY_W)) moveDir += fpCameraFront;
			if (isKeyPressed(GLFW_KEY_S)) moveDir -= fpCameraFront;
			if (isKeyPressed(GLFW_KEY_A)) moveDir -= glm::normalize(glm::cross(fpCameraFront, fpCameraUp));
			if (isKeyPressed(GLFW_KEY_D)) moveDir += glm::normalize(glm::cross(fpCameraFront, fpCameraUp));
			if (glm::length(moveDir) > 0.0f)
				fpCameraPos += fpCameraSpeed * glm::normalize(moveDir) * deltaT * 10.0f;

			// Mouse look: update yaw/pitch from mouse deltas
			const float mouseSensitivity = 0.1f;
			fpCameraYaw += mouseDeltaX * mouseSensitivity;
			fpCameraPitch -= mouseDeltaY * mouseSensitivity;
			// Clamp pitch
			if (fpCameraPitch > 89.0f) fpCameraPitch = 89.0f;
			if (fpCameraPitch < -89.0f) fpCameraPitch = -89.0f;
			// Update fpCameraFront from yaw/pitch
			float yawRad = glm::radians(fpCameraYaw);
			float pitchRad = glm::radians(fpCameraPitch);
			fpCameraFront.x = cos(pitchRad) * cos(yawRad);
			fpCameraFront.y = sin(pitchRad);
			fpCameraFront.z = cos(pitchRad) * sin(yawRad);
			fpCameraFront = glm::normalize(fpCameraFront);
			// Reset mouse deltas after use
			mouseDeltaX = 0.0;
			mouseDeltaY = 0.0;
		}

		if(glfwGetKey(window, GLFW_KEY_SPACE)) {
			if(!debounce && !firstPersonMode) {
				debounce = true;
				curDebounce = GLFW_KEY_SPACE;

				std::tuple<int,int,int> posKey = {static_cast<int>(Pos.x), static_cast<int>(Pos.y), static_cast<int>(Pos.z)};

				if(!placedObjects[posKey].empty()) {
					std::cout << "Removing object at: " << Pos.x << ", " << Pos.y << ", " << Pos.z << "\n";

					vkDeviceWaitIdle(device);
					SC.removeInstance(placedObjects[posKey]);
					placedObjects.erase(posKey);
				} else {
					
					std::cout << "Placing object at: " << Pos.x << ", " << Pos.y << ", " << Pos.z << "\n";

					std::string pId = objIds[selectedObj];
					float scale = 0.2f;
					auto sit = objectScale.find(pId);
					if(sit != objectScale.end()) scale = sit->second;

					float objRot = glm::half_pi<float>() * std::round(objectRotation / glm::half_pi<float>());

					glm::mat4 objTr = glm::translate(glm::mat4(1), Pos) *
										   glm::rotate(glm::mat4(1), objRot, glm::vec3(0,1,0)) *
										   glm::scale(glm::mat4(1), glm::vec3(scale));
					std::string id = pId + "_spawn_" + std::to_string(spawnCounter++);
					SC.addInstance(id, SC.MeshIds[pId], SC.TextureIds[pId], objTr, DSL);
					placedObjects[posKey] = id;
				}
			}
		} else if((curDebounce == GLFW_KEY_SPACE) && debounce) {
			debounce = false;
			curDebounce = 0;
		}

		if (glfwGetKey(window, GLFW_KEY_Q))
		{
			if (!debounce)
			{
				debounce = true;
				curDebounce = GLFW_KEY_Q;
				objectRotation -= glm::half_pi<float>();
				if (objectRotation < 0)
					objectRotation += glm::two_pi<float>();
			}
		}
		else if ((curDebounce == GLFW_KEY_Q) && debounce)
		{
			debounce = false;
			curDebounce = 0;
		}

		if (glfwGetKey(window, GLFW_KEY_E))
		{
			if (!debounce)
			{
				debounce = true;
				curDebounce = GLFW_KEY_E;
				objectRotation += glm::half_pi<float>();
				if (objectRotation > glm::two_pi<float>())
					objectRotation -= glm::two_pi<float>();
			}
		}
		else
		{
			if ((curDebounce == GLFW_KEY_E) && debounce)
			{
				debounce = false;
				curDebounce = 0;
			}
		}

		if (glfwGetKey(window, GLFW_KEY_N))
		{
			if (!debounce)
			{
				debounce = true;
				curDebounce = GLFW_KEY_N;
				//to wrap around the objIds vector
				selectedObj = (selectedObj + objIds.size() - 1) % objIds.size();
				std::cout << "Selected object: " << objIds[selectedObj] << "\n";

				InitialScale = glm::vec3(objectScale[objIds[selectedObj]]);
				SC.updateInstance("obj", SC.MeshIds[objIds[selectedObj]],
								  SC.TextureIds[objIds[selectedObj]], DSL);
				vkDeviceWaitIdle(device);
				vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()),
									 commandBuffers.data());
				createCommandBuffers();
			}
		}
		else
		{
			if ((curDebounce == GLFW_KEY_N) && debounce)
			{
				debounce = false;
				curDebounce = 0;
			}
		}

		if (glfwGetKey(window, GLFW_KEY_M))
		{
			if (!debounce)
			{
				debounce = true;
				curDebounce = GLFW_KEY_M;
				//updating selected object
				selectedObj = (selectedObj + 1) % objIds.size();
				std::cout << "Selected object: " << objIds[selectedObj] << "\n";

				InitialScale = glm::vec3(objectScale[objIds[selectedObj]]);
				SC.updateInstance("obj", SC.MeshIds[objIds[selectedObj]],
								  SC.TextureIds[objIds[selectedObj]], DSL);

				vkDeviceWaitIdle(device);
				vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()),
									 commandBuffers.data());
				createCommandBuffers();
			}
		}
		else
		{
			if ((curDebounce == GLFW_KEY_M) && debounce)
			{
				debounce = false;
				curDebounce = 0;
			}
		}
		
		// If the preview position is occupied, change the placed object's texture to red
		static bool placedRedTexture = false;
		static std::string lastRedId;
		static std::string lastRedBaseId;
		std::string currentId = placedObjects[std::make_tuple(Pos.x, Pos.y, Pos.z)];
		std::string currentBaseId;
		if (!currentId.empty() && (!placedRedTexture || lastRedId != currentId)) {
			//retrieving the base id of the object 
			size_t spawnPos = currentId.find("_spawn_");
			currentBaseId = currentId.substr(0, spawnPos);
			// Change previous red object to normal
			if (placedRedTexture && lastRedId != currentId) {
				SC.updateInstance(lastRedId, SC.MeshIds[lastRedBaseId], SC.TextureIds[lastRedBaseId], DSL);
			}
			// Set current object's texture to red
			SC.updateInstance(currentId, SC.MeshIds[currentBaseId], SC.TextureIds["red"], DSL);
			placedRedTexture = true;
			lastRedId = currentId;
			lastRedBaseId = currentBaseId;
			vkDeviceWaitIdle(device);
			vkFreeCommandBuffers(device, commandPool,
										static_cast<uint32_t>(commandBuffers.size()),
										commandBuffers.data());
			createCommandBuffers();
		} else if (currentId.empty() && placedRedTexture) {
			// Restore previous object texture
			SC.updateInstance(lastRedId, SC.MeshIds[lastRedBaseId], SC.TextureIds[lastRedBaseId], DSL);
			placedRedTexture = false;
			vkDeviceWaitIdle(device);
			vkFreeCommandBuffers(device, commandPool,
									static_cast<uint32_t>(commandBuffers.size()),
									commandBuffers.data());
			createCommandBuffers();
		}
	

		if (glfwGetKey(window, GLFW_KEY_C))
		{
			if (!debounce)
			{
				debounce = true;
				curDebounce = GLFW_KEY_C;
				firstPersonMode = !firstPersonMode;
			// Optionally reset camera position/orientation
				if (firstPersonMode) {
					fpCameraPos = glm::vec3(0.0f, 5.0f, 5.0f);
					fpCameraYaw = -90.0f;
					fpCameraPitch = 0.0f;
					mouseInitialized = false;
					glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

					//add ceiling
					glm::mat4 objTr = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 0.1f));
					SC.addInstance("top", SC.MeshIds["top"], SC.TextureIds["top"], objTr, DSL);
					
				} else {
					glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

					//remove ceiling
					vkDeviceWaitIdle(device);
					SC.removeInstance("top");
				}
				vkDeviceWaitIdle(device);
				vkFreeCommandBuffers(device, commandPool,
									static_cast<uint32_t>(commandBuffers.size()),
									commandBuffers.data());
				createCommandBuffers();
			}
		}
		else
		{
			if ((curDebounce == GLFW_KEY_C) && debounce)
			{
				debounce = false;
				curDebounce = 0;
			}
		}
		if(glfwGetKey(window, GLFW_KEY_LEFT)){
			if (!debounce)
			{
				debounce = true;
				curDebounce = GLFW_KEY_LEFT;
				invertedView = !invertedView;

			}
		}
		else{
			if ((curDebounce == GLFW_KEY_LEFT) && debounce)
			{
				debounce = false;
				curDebounce = 0;
			}
		}

		if(glfwGetKey(window, GLFW_KEY_RIGHT)){
			if (!debounce)
			{
				debounce = true;
				curDebounce = GLFW_KEY_RIGHT;
				invertedView = !invertedView;

			}
		}
		else{
			if ((curDebounce == GLFW_KEY_RIGHT) && debounce)
			{
				debounce = false;
				curDebounce = 0;
			}
		}

			// Save/load placed objects with K/L, delete all with J
		if (glfwGetKey(window, GLFW_KEY_K)) {
			if (!debounce) {
				debounce = true;
				curDebounce = GLFW_KEY_K;
				savePlacedObjects("placed_objects.txt");
			}
		} else {
			if ((curDebounce == GLFW_KEY_K) && debounce) {
				debounce = false;
				curDebounce = 0;
			}
		}
		if (glfwGetKey(window, GLFW_KEY_L)) {
			if (!debounce) {
				debounce = true;
				curDebounce = GLFW_KEY_L;
				loadPlacedObjects("placed_objects.txt");
			}
		} else {
			if ((curDebounce == GLFW_KEY_L) && debounce) {
				debounce = false;
				curDebounce = 0;
			}
		}
		if (glfwGetKey(window, GLFW_KEY_J)) {
			if (!debounce) {
				debounce = true;
				curDebounce = GLFW_KEY_J;
				std::remove("placed_objects.txt");
				loadPlacedObjects("placed_objects.txt");
			}
		} else {
			if ((curDebounce == GLFW_KEY_J) && debounce) {
				debounce = false;
				curDebounce = 0;
			}
		}

		if (glfwGetKey(window, GLFW_KEY_P))
		{
			if (!debounce)
			{
				debounce = true;
				curDebounce = GLFW_KEY_P;
				printVec3("Pos = ", Pos);
			}
		}
		else
		{
			if ((curDebounce == GLFW_KEY_P) && debounce)
			{
				debounce = false;
				curDebounce = 0;
			}
		}

		// Standard procedure to quit when the ESC key is pressed
		if(glfwGetKey(window, GLFW_KEY_ESCAPE)) {
			glfwSetWindowShouldClose(window, GL_TRUE);
		}


		UniformBufferObject ubo{};
		GlobalUniformBufferObject gubo{};
		glm::mat4 baseTr = glm::mat4(1.0f);
		glm::mat4 ViewPrj;
		glm::mat4 Mv;
		if (firstPersonMode) {
			// First person view, FOVy, aspect ratio, near/far planes
			glm::mat4 proj = glm::perspective(glm::radians(60.0f), Ar, 0.1f, 300.0f);
			proj[1][1] *= -1;
			Mv = glm::lookAt(fpCameraPos, fpCameraPos + fpCameraFront, fpCameraUp);
			ViewPrj = proj * Mv;
			gubo.eyePos = fpCameraPos;
		} else {
			glm::mat4 M = glm::perspective(glm::radians(60.0f), Ar, 0.1f, 500.0f);
			M[1][1] *= -1;
			glm::vec3 cameraOffset;
			if(!invertedView)
				cameraOffset= glm::vec3(-30.0f, 70.0f, 30.0f);
			else
				cameraOffset = glm::vec3(30.0f, 70.0f, -30.0f);
			cameraOffset = cameraOffset * glm::mat3(0.8f);
			glm::vec3 cameraPos = Pos * glm::vec3(1.0f, 0.0f, 1.0f) + cameraOffset;
			Mv = glm::lookAt(cameraPos, Pos, glm::vec3(0,1,0));
			ViewPrj = M * Mv;
			gubo.eyePos = glm::vec3(glm::inverse(Mv) * glm::vec4(0,0,0,1));
		}
		gubo.lightDir = glm::vec3(cos(glm::radians(45.0f)), sin(glm::radians(45.0f)), 0.0f);
		gubo.lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

		for(int i = 0; i < SC.InstanceCount; i++) {
			ubo.mMat = SC.I[i].Wm * baseTr;
			ubo.mvpMat = ViewPrj * ubo.mMat;
			ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));

			ubo.specularStrength = 1.0f;
            if(SC.I[i].id && (*SC.I[i].id == "floor" || *SC.I[i].id == "top" || *SC.I[i].id == "wall")) {
                ubo.specularStrength = 0.1f;
            }
			SC.DS[i]->map(currentImage, &ubo, sizeof(ubo), 0);
			SC.DS[i]->map(currentImage, &gubo, sizeof(gubo), 2);
		}
	}
};

// This is the main: probably you do not need to touch this!
int main() {
    ModularHospitalWardPlanner app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}