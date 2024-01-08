#include <glm/fwd.hpp>
#include <vector>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

// EventClick-> Check location against panes
// if in pane Pane will move by MousePos Delta for everytime that left mouse evaluates to GLFW_PRESS
// else ignore the event

// Widget::Position is the position of the top left corner of the widget

/* Widgets */
class Widget
{
  public:
    uint32_t Width;
    uint32_t Height;

    glm::vec2 Position;

    bool isCollide(glm::vec2 Position);
};

class ColorPane : Widget
{
  public:
    void Init(uint32_t PosX, uint32_t PosY, uint32_t Width, uint32_t Height, glm::vec3 Color);

    glm::vec3 Color;
};

class UIInterface
{
  public:
    GLFWwindow* Window;
    VkExtent2D WindowSize;

    std::vector<Widget> Widgets;
};

class UIRenderer : UIInterface
{
  public:
    void Init(VkInstance& VkInstance, VkPhysicalDevice& PhysicalDevice, VkDevice& Device, VkRenderPass& RenderPass, uint32_t uiSubpass);
    void SetOutput(GLFWwindow* Window, VkSurfaceKHR& Surface, VkSwapchainKHR& Swapchain);

  private:
    void CreatePipeline();

    /* vulkan stuff */
    VkInstance* pInstance;
    VkPhysicalDevice* pPhysicalDevice;
    VkDevice* pDevice;

    VkRenderPass* pRenderpass;
    uint32_t Subpass;

    VkSwapchainKHR* pSwapchain;

    VkShaderModule Vertex;
    VkShaderModule Fragment;
    VkPipeline uiPipe;
};

