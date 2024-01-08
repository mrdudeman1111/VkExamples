Changes:
    InitPipeline()
        - Added Descriptor Layout
        - Pipeline Layout Creation
    InitDescriptorPool()
        - Descriptor size
    InitDescriptors()
        - Descriptor Initiation added for sampler
Added:
    Variables:
    Mesh::Image_Albedo
    Mesh::View_Albedo
    Mesh::Memory_Albedo
    Mesh::Sampler_Albedo
    Mesh::TextureDescriptor
    
Helper functions:
    ::CreateImage()
    ::CreateView()

Creation functions:
    ::InitSamplers()
    ::InitBuffers()  # moving buffer creation out of InitMesh() to remove clutter


 We can keep the WVP structure and descriptor in the Context Structure because it is a global constant, Because of this, we have One Buffer for the MVP for ALL render operations.
But with the Texture sampler in the mesh descriptor, it will change depending on the mesh. So the actual image and image memory will be an attribute of the Mesh and each mesh will have it's own 
texture, so it can be dynamic. We contain the Texture in the Mesh structure so it will be easier to implement multi-mesh rendering.

 Uniforms and samplers tend to have a "set = #" in their location() parameters. This refers to the DescriptorLayout Index in the pSetLayouts array of your pipeline layout create info.

 Samplers are different than Images/Textures, the sampler is just a structure defining the read techniques to use. Like how to blend pixel values together, and what to do when you read out of bounds
pixels (like if we read 2048x1080 out of a 1080x1080 image). So it is not an Image, it is a set of instructions on how to read an image. 
 In shaders, you need a sampler, and an image to make a texture. This way you can read the image with filtering, so that means we can upload the textures as an Image and a sampler, or a combined image sampler.

 For texture importing we will use SIMP.
