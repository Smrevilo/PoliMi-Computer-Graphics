#ifndef ICON_MAKER_HPP
#define ICON_MAKER_HPP

#include "Starter.hpp"
#include <unordered_map>
#include <string>

struct IconVertex {
        glm::vec2 pos;
        glm::vec2 texCoord;
};

struct IconMaker {
        VertexDescriptor VD;
        DescriptorSetLayout DSL;
        Pipeline P;
        BaseProject *BP;

        struct Icon {
                Model<IconVertex> M;
                Texture T;
                DescriptorSet DS;
        };

        std::unordered_map<std::string, Icon> Icons;
        float startX;
        float startY;
        float dx;
        float dy;

        void init(BaseProject *bp, const std::vector<std::pair<std::string,std::string>> &files,
                        uint32_t windowWidth, uint32_t windowHeight) {
                BP = bp;
                VD.init(BP, {
                                  {0, sizeof(IconVertex), VK_VERTEX_INPUT_RATE_VERTEX}
                                }, {
                                  {0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(IconVertex,pos), sizeof(glm::vec2), OTHER},
                                  {0, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(IconVertex,texCoord), sizeof(glm::vec2), UV}
                                });
                DSL.init(BP, {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}});
                P.init(BP, &VD, "shaders/TextVert.spv", "shaders/TextFrag.spv", {&DSL});
                P.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, false);
                P.create();

                dx = 2.0f * 64.0f / windowWidth;
                dy = 2.0f * 64.0f / windowHeight;
                startX = 0.95f - dx;
                startY = 0.95f - dy;

                for(auto &p : files) {
                        Icon I;
                        createQuad(I.M);
                        I.M.initMesh(BP, &VD);
                        I.T.init(BP, p.second);
                        I.DS.init(BP, &DSL, {{0, TEXTURE, 0, &I.T}});
                        Icons[p.first] = I;
                }
        }

        void createQuad(Model<IconVertex> &M) {
                M.vertices = {
                        {{startX, startY}, {0.0f, 0.0f}},
                        {{startX + dx, startY}, {1.0f, 0.0f}},
                        {{startX, startY + dy}, {0.0f, 1.0f}},
                        {{startX + dx, startY + dy}, {1.0f, 1.0f}}
                };
                M.indices = {0,1,2,1,2,3};
        }

        void pipelinesAndDescriptorSetsInit() {
                P.create();
                for(auto &kv : Icons) {
                        kv.second.DS.init(BP, &DSL, {{0, TEXTURE, 0, &kv.second.T}});
                }
        }

        void pipelinesAndDescriptorSetsCleanup() {
                P.cleanup();
                for(auto &kv : Icons) kv.second.DS.cleanup();
        }

        void localCleanup() {
                for(auto &kv : Icons) {
                        kv.second.T.cleanup();
                        kv.second.M.cleanup();
                }
                DSL.cleanup();
                P.destroy();
        }

        void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage, const std::string &id) {
                auto it = Icons.find(id);
                if(it == Icons.end()) return;
                P.bind(commandBuffer);
                it->second.M.bind(commandBuffer);
                it->second.DS.bind(commandBuffer, P, 0, currentImage);
                vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(it->second.M.indices.size()), 1, 0, 0, 0);
        }
};

#endif