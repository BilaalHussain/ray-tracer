-- A simple scene with five spheres

mat1 = gr.material({0.7, 1.0, 0.7}, {0.5, 0.7, 0.5}, 25)
mat2 = gr.material({0.5, 0.5, 0.5}, {0.5, 0.7, 0.5}, 25)
mat3 = gr.material({1.0, 0.6, 0.1}, {0.5, 0.7, 0.5}, 25)
mat4 = gr.material({.4,.4, .4}, {0.1, 0.1, 0.1}, 2)

scene_root = gr.node('root')

s1 = gr.nh_sphere('s1', {0, 3, -7}, 4)
scene_root:add_child(s1)
s1:set_material(mat3)

s2 = gr.nh_sphere('s2', {0, 0, -2}, 1)
scene_root:add_child(s2)
s2:set_material(mat4)

white_light = gr.light({300.0, 0, -100.0}, {0.9, 0.9, 0.9}, {1, 0, 0})
magenta_light = gr.light({-1000.0, 0, -400.0}, {0.7, 0.9, 0.7}, {1, 0, 0})

gr.render(scene_root, 'refract.png', 256, 256,
	  {0, 0, 1}, {0, 0, -1}, {0, 1, 0}, 75,
	  {0.2, 0.2, 0.2}, {white_light})
