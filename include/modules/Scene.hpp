
typedef struct {
	std::string *id;
	int Mid;
	int Tid;
	glm::mat4 Wm;
} Instance;

class Scene {
	public:
	VertexDescriptor *VD;	
	
	BaseProject *BP;

	// Models, textures and Descriptors (values assigned to the uniforms)
	// Please note that Model objects depends on the corresponding vertex structure
	// Models
	int ModelCount = 0;
	Model<Vertex> **M;
	std::unordered_map<std::string, int> MeshIds;

	// Textures
	int TextureCount = 0;
	Texture **T;
	std::unordered_map<std::string, int> TextureIds;
	
	// Descriptor sets and instances
	int InstanceCount = 0;
	DescriptorSet **DS;
	Instance *I;
	std::unordered_map<std::string, int> InstanceIds;

	void init(BaseProject *_BP, VertexDescriptor *VD, DescriptorSetLayout &DSL, 
			  Pipeline &P, std::string file) {
		BP = _BP;
		// Models, textures and Descriptors (values assigned to the uniforms)
		nlohmann::json js;
		std::ifstream ifs("assets/models/scene.json");
		if (!ifs.is_open()) {
		  std::cout << "Error! Scene file not found!";
		  exit(-1);
		}
		try {
			std::cout << "Parsing JSON\n";
			ifs >> js;
			ifs.close();
			std::cout << "\n\n\nScene contains " << js.size() << " definitions sections\n\n\n";
			
			// MODELS
			nlohmann::json ms = js["models"];
			ModelCount = ms.size();
			std::cout << "Models count: " << ModelCount << "\n";

			M = (Model<Vertex> **)calloc(ModelCount, sizeof(Model<Vertex> *));
			for(int k = 0; k < ModelCount; k++) {
				MeshIds[ms[k]["id"]] = k;
				std::string MT = ms[k]["format"].template get<std::string>();
				M[k] = new Model<Vertex>();

				M[k]->init(BP, VD, ms[k]["model"], (MT[0] == 'O') ? OBJ : ((MT[0] == 'G') ? GLTF : MGCG));
			}
			
			// TEXTURES
			nlohmann::json ts = js["textures"];
			TextureCount = ts.size();
			std::cout << "Textures count: " << TextureCount << "\n";

                        T = (Texture **)calloc(TextureCount, sizeof(Texture *));
			for(int k = 0; k < TextureCount; k++) {
				TextureIds[ts[k]["id"]] = k;
				T[k] = new Texture();

				T[k]->init(BP, ts[k]["texture"].template get<std::string>());
			}

			// INSTANCES TextureCount
			nlohmann::json is = js["instances"];
			InstanceCount = is.size();
			std::cout << "Instances count: " << InstanceCount << "\n";

			DS = (DescriptorSet **)calloc(InstanceCount, sizeof(DescriptorSet *));
			I =  (Instance *)calloc(InstanceCount, sizeof(Instance));
			for(int k = 0; k < InstanceCount; k++) {
std::cout << k << "\t" << is[k]["id"] << ", " << is[k]["model"] << "(" << MeshIds[is[k]["model"]] << "), " << is[k]["texture"] << "(" << TextureIds[is[k]["texture"]] << ")\n";
				InstanceIds[is[k]["id"]] = k;
				I[k].id  = new std::string(is[k]["id"]);
				I[k].Mid = MeshIds[is[k]["model"]];
				I[k].Tid = TextureIds[is[k]["texture"]];
				nlohmann::json TMjson = is[k]["transform"];
				float TMj[16];
				for(int l = 0; l < 16; l++) {TMj[l] = TMjson[l];}
				I[k].Wm = glm::mat4(TMj[0],TMj[4],TMj[8],TMj[12],TMj[1],TMj[5],TMj[9],TMj[13],TMj[2],TMj[6],TMj[10],TMj[14],TMj[3],TMj[7],TMj[11],TMj[15]);
			}			
		} catch (const nlohmann::json::exception& e) {
			std::cout << e.what() << '\n';
		}
	}


	void pipelinesAndDescriptorSetsInit(DescriptorSetLayout &DSL) {
		for(int i = 0; i < InstanceCount; i++) {
			DS[i] = new DescriptorSet();
			DS[i]->init(BP, &DSL, {
							{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
							{1, TEXTURE, 0, T[I[i].Tid]},
							{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
					});
		}
	}

	int addInstance(const std::string &id, int meshIdx, int texIdx,
					const glm::mat4 &transform, DescriptorSetLayout &DSL) {
		int idx = InstanceCount;
		InstanceCount++;
		I = (Instance *)realloc(I, InstanceCount * sizeof(Instance));
		DS = (DescriptorSet **)realloc(DS, InstanceCount * sizeof(DescriptorSet *));

		I[idx].id = new std::string(id);
		I[idx].Mid = meshIdx;
		I[idx].Tid = texIdx;
		I[idx].Wm = transform;

		DS[idx] = new DescriptorSet();
		DS[idx]->init(BP, &DSL, {
								{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
								{1, TEXTURE, 0, T[texIdx]},
								{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
						});

		InstanceIds[id] = idx;
		return idx;
	}

	void removeInstance(const std::string &id) {
		auto it = InstanceIds.find(id);
		if(it == InstanceIds.end()) return;
		int idx = it->second;
		int last = InstanceCount - 1;

		DescriptorSet* toDelete = DS[idx];
		toDelete->cleanup();
		delete toDelete;
		delete I[idx].id;

		if(idx != last) {
			DS[idx] = DS[last];
			I[idx] = I[last];
			InstanceIds[*I[idx].id] = idx;
		}

		InstanceIds.erase(it);
		InstanceCount--;
		DS = (DescriptorSet **)realloc(DS, InstanceCount * sizeof(DescriptorSet *));
		I  = (Instance *)realloc(I, InstanceCount * sizeof(Instance));
	}
	
	void pipelinesAndDescriptorSetsCleanup() {
		// Cleanup datasets
		for(int i = 0; i < InstanceCount; i++) {
			DS[i]->cleanup();
			delete DS[i];
			//delete DS[i];
		}
	}

	void localCleanup() {
		// Cleanup textures
		for(int i = 0; i < TextureCount; i++) {
			T[i]->cleanup();
			delete T[i];
		}
		free(T);
		
		// Cleanup models
		for(int i = 0; i < ModelCount; i++) {
			M[i]->cleanup();
			delete M[i];
		}
		free(M);
		
		free(DS);
		for(int i = 0; i < InstanceCount; i++) {
			delete I[i].id;
		}
		free(I);
	}
	
    void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage, Pipeline &P) {
		for(int i = 0; i < InstanceCount; i++) {
			M[I[i].Mid]->bind(commandBuffer);
			DS[i]->bind(commandBuffer, P, 0, currentImage);
						
			vkCmdDrawIndexed(commandBuffer,
					static_cast<uint32_t>(M[I[i].Mid]->indices.size()), 1, 0, 0, 0);
		}
	}
};
    
