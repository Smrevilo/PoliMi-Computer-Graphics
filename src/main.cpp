// This has been adapted from the Vulkan tutorial

// TO MOVE
#define JSON_DIAGNOSTICS 1
#include "modules/Starter.hpp"
#include "modules/TextMaker.hpp"
#include "modules/IconMaker.hpp"
#include <cmath>
#include <glm/gtc/constants.hpp>
#include <unordered_map>

// The uniform buffer object used in this example
struct UniformBufferObject {
	alignas(16) glm::mat4 mvpMat;
	alignas(16) glm::mat4 mMat;
	alignas(16) glm::mat4 nMat;
};

struct GlobalUniformBufferObject {
	alignas(16) glm::vec3 lightDir;
	alignas(16) glm::vec4 lightColor;
	alignas(16) glm::vec3 eyePos;
	alignas(16) glm::vec4 eyeDir;
};



// The vertices data structures
// Example
struct Vertex {
	glm::vec3 pos;
	glm::vec2 UV;
	glm::vec3 norm;
};

#include "modules/Scene.hpp"

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

	// Models
	Model<Vertex> M1;
	// Descriptor sets
	DescriptorSet DS1;

	Scene SC;
	IconMaker IR;

	// Other application parameters
	float Ar;
	glm::vec3 Pos;
	glm::vec3 InitialPos;
	glm::vec3 InitialScale;
	float objectRotation = 0.0f;

	std::unordered_map<std::tuple<int,int,int>, std::string, GridCoordHash> placedObjects;
	int spawnCounter = 0;
	std::vector<std::string> objIds;
	int selectedObj = 0;
	std::unordered_map<std::string, float> objectScale;

	// Here you set the main application parameters
	void setWindowParameters() {
		// window size, titile and initial background
		windowWidth = 800;
		windowHeight = 600;
                windowTitle = "Modular Hospital Ward Planner";
    	windowResizable = GLFW_TRUE;
		initialBackgroundColor = {0.0f, 0.0f, 0.0f, 1.0f};

		// Descriptor pool sizes
		// allow for many dynamically spawned objects
		uniformBlocksInPool = 2000;
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

		// Models, textures and Descriptors (values assigned to the uniforms)
		M1.vertices = {{{-100.0,0.0f,-100.0}, {0.45f,0.1f}, {0.0f,1.0f,0.0f}},
					   {{-100.0,0.0f, 100.0}, {0.45f,0.1f}, {0.0f,1.0f,0.0f}},
					   {{ 100.0,0.0f,-100.0}, {0.45f,0.1f}, {0.0f,1.0f,0.0f}},
					   {{ 100.0,0.0f, 100.0}, {0.45f,0.1f}, {0.0f,1.0f,0.0f}}};
		M1.indices = {0, 0, 0,    0, 0, 0};
		M1.initMesh(this, &VD);


		// Load Scene
		SC.init(this, &VD, DSL, P, "assets/models/scene.json");

		// Init local variables
		Pos = glm::vec3(SC.I[SC.InstanceIds["obj"]].Wm[3]);
		InitialPos = Pos;

		objIds = {"potted1", "potted2",
                             "aircondition", "bed", "bulletinboard", "cabinet",
                             "closestool", "curtain", "door1", "door2", "door3",
                             "nursesstation", "pc", "poster", "shelf", "socket",
                             "sofa", "tv", "top", "trashcan", "wardrobe", "window"};

		objectScale = {{"potted1",0.2f},{"potted2",0.2f},{"aircondition",0.2f},
		{"bed",0.02f},{"bulletinboard",0.2f},{"cabinet",0.2f},{"closestool",0.2f},
		{"curtain",0.2f},{"door1",0.2f},{"door2",0.2f},{"door3",0.2f},{"nursesstation",0.2f},
		{"pc",0.2f},{"poster",0.2f},{"shelf",0.2f},{"socket",0.2f},{"sofa",0.2f},{"tv",0.2f},
		{"top",0.2f},{"trashcan",0.2f},{"wardrobe",0.2f},{"window",0.2f}};

		InitialScale = glm::vec3(objectScale[objIds[selectedObj]]);

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
                      {"top", "assets/Icons/M_Top_01.png"},
                      {"trashcan", "assets/Icons/M_TrashCan_01.png"},
                      {"wardrobe", "assets/Icons/M_Wardrobe_01.png"},
                      {"window", "assets/Icons/M_Window_01.png"}},
                     windowWidth, windowHeight);
	}

	// Here you create your pipelines and Descriptor Sets!
	void pipelinesAndDescriptorSetsInit() {
		// This creates a new pipeline (with the current surface), using its shaders
		P.create();

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
		// Cleanup pipelines
		P.cleanup();
		DS1.cleanup();

		SC.pipelinesAndDescriptorSetsCleanup();
		IR.pipelinesAndDescriptorSetsCleanup();
	}

	// Here you destroy all the Models, Texture and Desc. Set Layouts you created!
	// All the object classes defined in Starter.hpp have a method .cleanup() for this purpose
	// You also have to destroy the pipelines: since they need to be rebuilt, they have two different
	// methods: .cleanup() recreates them, while .destroy() delete them completely
	void localCleanup() {
		// Cleanup descriptor set layouts
		DSL.cleanup();

		// Destroies the pipelines
		P.destroy();

		// Cleanup models
		M1.cleanup();

		SC.localCleanup();
		IR.localCleanup();
	}

	// Here it is the creation of the command buffer:
	// You send to the GPU all the objects you want to draw,
	// with their buffers and textures

	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
		// binds the pipeline
		P.bind(commandBuffer);

		// binds the data set
		DS1.bind(commandBuffer, P, 0, currentImage);

		// binds the model
		M1.bind(commandBuffer);

		// record the drawing command in the command buffer
		vkCmdDrawIndexed(commandBuffer,
				static_cast<uint32_t>(M1.indices.size()), 1, 0, 0, 0);

		SC.populateCommandBuffer(commandBuffer, currentImage, P);
		IR.populateCommandBuffer(commandBuffer, currentImage, objIds[selectedObj]);
	}

	// Here is where you update the uniforms.
	// Very likely this will be where you will be writing the logic of your application.
	void updateUniformBuffer(uint32_t currentImage) {
		static bool debounce = false;
		static int curDebounce = 0;

		float deltaT;
		glm::vec3 m = glm::vec3(0.0f), r = glm::vec3(0.0f);
		bool fire = false;
		getSixAxis(deltaT, m, r, fire);

		const float ROT_SPEED = glm::radians(360.0f);
		const float MOVE_SPEED = 10.0f;
		int objectId = SC.InstanceIds["obj"];
		static float lookAng = 0;
		static float DlookAng = 0;
		static int subpass = 0;
		static float subpassTimer = 0.0;

		m.y = 0;
		m = glm::vec3(glm::rotate(glm::mat4(1), DlookAng, glm::vec3(0,1,0)) * glm::vec4(m, 1));
		Pos += m * MOVE_SPEED * deltaT;

		lookAng -= r.y * ROT_SPEED * deltaT;
		lookAng = (lookAng < -3.1416) ? lookAng + 2*3.1416 : ((lookAng > 3.1416) ? lookAng - 2*3.1416 : lookAng);
		DlookAng = lookAng;

		// Discrete movement using WASD + shift + ctrl keys
		static bool wKey = false, aKey = false, sKey = false, dKey = false, shift = false, ctrl = false;
		constexpr float GRID_SIZE = 2.0f;
		float snappedAngMove = glm::half_pi<float>() * std::round(DlookAng / glm::half_pi<float>());
		glm::vec3 forwardDir = glm::vec3(glm::rotate(glm::mat4(1), snappedAngMove, glm::vec3(0,1,0)) * glm::vec4(0,0,-1.0f,0));
		glm::vec3 rightDir   = glm::vec3(glm::rotate(glm::mat4(1), snappedAngMove, glm::vec3(0,1,0)) * glm::vec4(1.0f,0,0,0));
		glm::vec3 upDir = glm::vec3(glm::rotate(glm::mat4(1), snappedAngMove, glm::vec3(0,1,0)) * glm::vec4(0, 1.0f, 0, 0));

		static float movementTimer = 0.0f;
		const float MOVEMENT_DIFF = 0.08f;
		//the flag is not used, but i kept it so that it can be re-implemented if needed
		auto stepMove = [&](int key, bool &flag, const glm::vec3 &dir) {
			if(glfwGetKey(window, key) && movementTimer <= 0.0f) {
				movementTimer = MOVEMENT_DIFF;
				if(!flag) {
					//flag = true;
					glm::vec3 newPos = Pos + dir * GRID_SIZE;
					int gx = static_cast<int>(std::round(newPos.x / GRID_SIZE));
					int gz = static_cast<int>(std::round(newPos.z / GRID_SIZE));
					int gy = static_cast<int>(std::round(newPos.y / GRID_SIZE));
					newPos.x = GRID_SIZE * gx;
					newPos.z = GRID_SIZE * gz;
					newPos.y = GRID_SIZE * gy;
					if(newPos.y < 0.0f) newPos.y = 0.0f;
					Pos = newPos;
				}
			} else {
				if(flag) flag = false;
			}
		};
		movementTimer -= deltaT;
		if(movementTimer < 0.0f) movementTimer = 0.0f;

		stepMove(GLFW_KEY_W, wKey,  forwardDir);
		stepMove(GLFW_KEY_S, sKey, -forwardDir);
		stepMove(GLFW_KEY_A, aKey, -rightDir);
		stepMove(GLFW_KEY_D, dKey,  rightDir);
		stepMove(GLFW_KEY_LEFT_SHIFT, shift,  upDir);
		stepMove(GLFW_KEY_LEFT_CONTROL, ctrl,  -upDir);

		// Update object position so it matches the placement logic
		float snappedAng = glm::half_pi<float>() * std::round(DlookAng / glm::half_pi<float>());
		glm::vec3 forward = glm::vec3(glm::rotate(glm::mat4(1), snappedAng, glm::vec3(0,1,0)) * glm::vec4(0,0,-1.0f,0));
		glm::vec3 objectPos = Pos + forward * GRID_SIZE;
		int ggx = static_cast<int>(std::round(objectPos.x / GRID_SIZE));
		int ggz = static_cast<int>(std::round(objectPos.z / GRID_SIZE));
		int ggy = static_cast<int>(std::round(objectPos.y / GRID_SIZE));
		objectPos.x = GRID_SIZE * ggx;
		objectPos.z = GRID_SIZE * ggz;
		objectPos.y = GRID_SIZE * ggy;
		if(objectPos.y < 0.0f) objectPos.y = 0.0f;

		float objectRot = glm::half_pi<float>() * std::round(objectRotation / glm::half_pi<float>());
		SC.I[objectId].Wm = glm::translate(glm::mat4(1), objectPos) * glm::rotate(glm::mat4(1), objectRot, glm::vec3(0,1,0)) * glm::scale(glm::mat4(1), InitialScale);

		if(glfwGetKey(window, GLFW_KEY_SPACE)) {
			if(!debounce) {
				debounce = true;
				curDebounce = GLFW_KEY_SPACE;

				constexpr float GRID_SIZE = 2.0f;
				float snappedAng = glm::half_pi<float>() * std::round(DlookAng / glm::half_pi<float>());

				glm::vec3 forward = glm::vec3(glm::rotate(glm::mat4(1), snappedAng, glm::vec3(0,1,0)) *
												  glm::vec4(0,0,-1.0f,0));
				glm::vec3 placePos = Pos + forward * GRID_SIZE;
				int gx = static_cast<int>(std::round(placePos.x / GRID_SIZE));
				int gz = static_cast<int>(std::round(placePos.z / GRID_SIZE));
				int gy = static_cast<int>(std::round(placePos.y / GRID_SIZE));
				placePos.x = GRID_SIZE * gx;
				placePos.z = GRID_SIZE * gz;
				placePos.y = GRID_SIZE * gy;
				std::cout << "Placing object at: " << placePos.x << ", " << placePos.y << ", " << placePos.z << "\n";
				std::tuple<int,int,int> gkey = {gx, gz, gy};

				auto pit = placedObjects.find(gkey);
				if(pit != placedObjects.end()) {
					SC.removeInstance(pit->second);
					placedObjects.erase(pit);
				} else {
					std::string pId = objIds[selectedObj];
					float scale = 0.2f;
					auto sit = objectScale.find(pId);
					if(sit != objectScale.end()) scale = sit->second;

					float objRot = glm::half_pi<float>() * std::round(objectRotation / glm::half_pi<float>());

					glm::mat4 objTr = glm::translate(glm::mat4(1), placePos) *
										   glm::rotate(glm::mat4(1), objRot, glm::vec3(0,1,0)) *
										   glm::scale(glm::mat4(1), glm::vec3(scale));
					std::string id = pId + "_spawn_" + std::to_string(spawnCounter++);
					SC.addInstance(id, SC.MeshIds[pId], SC.TextureIds[pId], objTr, DSL);
					placedObjects[gkey] = id;
				}
				// Re-record command buffers so the new instance
				// is included in the rendering pipeline
				vkDeviceWaitIdle(device);
				vkFreeCommandBuffers(device, commandPool,
									static_cast<uint32_t>(commandBuffers.size()),
									commandBuffers.data());
				createCommandBuffers();
			}
		} else {
			if((curDebounce == GLFW_KEY_SPACE) && debounce) {
				debounce = false;
				curDebounce = 0;
			}
		}

		if(glfwGetKey(window, GLFW_KEY_Q)) {
                        if(!debounce) {
                                debounce = true;
                                curDebounce = GLFW_KEY_Q;
                                objectRotation -= glm::half_pi<float>();
                                if(objectRotation < 0)
                                        objectRotation += glm::two_pi<float>();
                        }
                } else {
                        if((curDebounce == GLFW_KEY_Q) && debounce) {
                                debounce = false;
                                curDebounce = 0;
                        }
                }

                if(glfwGetKey(window, GLFW_KEY_E)) {
                        if(!debounce) {
                                debounce = true;
                                curDebounce = GLFW_KEY_E;
                                objectRotation += glm::half_pi<float>();
                                if(objectRotation > glm::two_pi<float>())
                                        objectRotation -= glm::two_pi<float>();
                        }
                } else {
                        if((curDebounce == GLFW_KEY_E) && debounce) {
                                debounce = false;
                                curDebounce = 0;
                        }
                }

                if(glfwGetKey(window, GLFW_KEY_N)) {
                        if(!debounce) {
                                debounce = true;
                                curDebounce = GLFW_KEY_N;
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
                } else {
                	if((curDebounce == GLFW_KEY_N) && debounce) {
                		debounce = false;
                		curDebounce = 0;
                	}
                }

		if(glfwGetKey(window, GLFW_KEY_M)) {
			if(!debounce) {
				debounce = true;
				curDebounce = GLFW_KEY_M;
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
		} else {
			if((curDebounce == GLFW_KEY_M) && debounce) {
				debounce = false;
				curDebounce = 0;
			}
		}

		if(glfwGetKey(window, GLFW_KEY_P)) {
			if(!debounce) {
				debounce = true;
				curDebounce = GLFW_KEY_P;
				printVec3("Pos = ", Pos);
			}
		} else {
			if((curDebounce == GLFW_KEY_P) && debounce) {
				debounce = false;
				curDebounce = 0;
			}
		}
		// Standard procedure to quit when the ESC key is pressed
		if(glfwGetKey(window, GLFW_KEY_ESCAPE)) {
			glfwSetWindowShouldClose(window, GL_TRUE);
		}


		glm::mat4 M = glm::perspective(glm::radians(60.0f), Ar, 0.1f, 500.0f);
		M[1][1] *= -1;

		glm::vec3 cameraOffset = glm::vec3(-35.0f, 70.0f, 35.0f);
		glm::vec3 cameraPos = Pos + cameraOffset;
		glm::mat4 Mv = glm::lookAt(cameraPos, Pos, glm::vec3(0,1,0));

		glm::mat4 ViewPrj =  M * Mv;
		UniformBufferObject ubo{};
		glm::mat4 baseTr = glm::mat4(1.0f);

		// updates global uniforms
		GlobalUniformBufferObject gubo{};
		gubo.lightDir = glm::vec3(cos(glm::radians(135.0f)), sin(glm::radians(135.0f)), 0.0f);
		gubo.lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		gubo.eyePos = glm::vec3(glm::inverse(Mv) * glm::vec4(0,0,0,1));
		gubo.eyeDir = glm::vec4(0);

		for(int i = 0; i < SC.InstanceCount; i++) {
			ubo.mMat = SC.I[i].Wm * baseTr;
			ubo.mvpMat = ViewPrj * ubo.mMat;
			ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));

			SC.DS[i]->map(currentImage, &ubo, sizeof(ubo), 0);
			SC.DS[i]->map(currentImage, &gubo, sizeof(gubo), 2);
		}

		ubo.mMat = baseTr;
		ubo.mvpMat = ViewPrj * ubo.mMat;
		ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));
		DS1.map(currentImage, &ubo, sizeof(ubo), 0);
		DS1.map(currentImage, &gubo, sizeof(gubo), 2);
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