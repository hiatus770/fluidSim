# Fluid simulation Notes

Navier Stokes Equations: 
- Continuity Equation
$$ \nabla \cdot u = 0 $$
- Momentum Equation
$$ \frac{\partial u}{\partial t} + u \cdot \nabla u = -\frac{1}{\rho} \nabla p + \nu \nabla^2 u + f $$

- This is used to describe all fluids 
- The F is the external force of gravity and other obstacles such as walls 

We use a grid to appproximate an infinite amount of points, we represent it as a grid or particles and this implementation uses particles

Can use levels of detail if you want to go more for performance

Given one set of velocity and other information, we are going to calculate every other thing from that 

## Step one: Diffusion
- This is the process of particles moving from high concentration to low concentration
- We make each cell of the grid gradually become the average of the other values

- d(x,y) = density at coordinate x,y
- s(x,y) = source density at coordinate x,y
- $$ s(x,y) = \frac{d(x-1,y) + d(x+1,y) + d(x,y-1) + d(x,y+1))}{4} $$
- To find the next state of the next diffusion (D_NEXT) we have to use (D_CURRENT)

$$ d_{next} = \frac{d_{current} + k \cdot s_n}{1 + k} $$
$$ d_n (x,y) = \frac{d_{current} + k \cdot s(x,y)}{1+k}$$
$$ d_n (x,y) = \frac{d_{current} + k \cdot \frac{d(x-1,y) + d(x+1,y) + d(x,y-1) + d(x,y+1))}{4}}{1+k}$$

This is a system of simultaneous equations because we need to konw s(x,y) but s(x,y) requires knowing d(x, y+1) which means we need to know d(x,y), so we need to solve this system of equations

$$ d_n (x,y) = \frac{d_{current} + k \cdot \frac{d(x-1,y) + d(x+1,y) + d(x,y-1) + d(x,y+1))}{4}}{1+k}$$

We can use the Gauss-Siedel method to do this and its kinda wild 

The gauss siedel method is the one used in the MikeAsh article, he doesn't know what it is but he does end up using it for his solving method! 

To linear solve all ofthese, we have a set amount of iterations and we set the initials values to all be random 

#### Linear solvers
- Gauss Siedel
    - Works by solving one equation at a time, and then using the new value to solve the next equation
    - It then converges to the solution of the equations
- Multigrid 
- Conjugate Gradient
- Fast Fourier Transform





# Articles / Links
https://www.youtube.com/watch?v=x9xPX3WiK3E
https://www.youtube.com/watch?v=iKAVRgIrUOU&ab_channel=TenMinutePhysics
https://paveldogreat.github.io/WebGL-Fluid-Simulation/
http://graphics.cs.cmu.edu/nsp/course/15-464/Fall09/papers/StamFluidforGames.pdf

