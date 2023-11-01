# Real Time Fluid Simulation

Navier Stokes Equations: 
- Continuity Equation
$` \nabla \cdot u = 0 `$
- Momentum Equation
$` \frac{\partial u}{\partial t} + u \cdot \nabla u = -\frac{1}{\rho} \nabla p + \nu \nabla^2 u + f `$

- This is used to describe all fluids 
- The F is the external force of gravity and other obstacles such as walls 

We use a grid to appproximate an infinite amount of points, we represent it as a grid or particles and this implementation uses particles

Can use levels of detail if you want to go more for performance

Given one state of information, our job is to predict the next set 

## Step one: Diffusion
- This is the process of particles moving from high concentration to low concentration
- We make each cell of the grid gradually become the average of the other values
- d(x,y) = density at coordinate x,y
- s(x,y) = source density at coordinate x,y
- $` s(x,y) = \frac{d(x-1,y) + d(x+1,y) + d(x,y-1) + d(x,y+1))}{4} `$
- To find the next state of the next diffusion (D_NEXT) we have to use (D_CURRENT)

$$ d_{next} = \frac{d_{current} + k \cdot s_n}{1 + k} $$
$$ d_n (x,y) = \frac{d_{current} + k \cdot s(x,y)}{1+k}$$
$$ d_n (x,y) = \frac{d_{current} + k \cdot \frac{d(x-1,y) + d(x+1,y) + d(x,y-1) + d(x,y+1))}{4}}{1+k}$$

This is a system of simultaneous equations because we need to konw s(x,y) but s(x,y) requires knowing d(x, y+1) which means we need to know d(x,y), so we need to solve this system of equations

$$ d_n (x,y) = \frac{d_{current} + k \cdot \frac{d(x-1,y) + d(x+1,y) + d(x,y-1) + d(x,y+1))}{4}}{1+k}$$

We can use the Gauss-Siedel method to do this ( $` \mathcal O(N^2) `$  ) where N is resolution of the simulation , faster methods exist such as the Multi-Grid algorithm which is $` \mathcal O(n) `$ 

The gauss siedel method is the one used in the MikeAsh article and the Jos Stam Paper. 

To linear solve all ofthese, we have a set amount of iterations and we set the initials values to all be random 

### Linear solvers
- Gauss Siedel
    - Works by solving one equation at a time, and then using the new value to solve the next equation
    - It then converges to the solution of the equations over time, I foudn that usualy 40 iterations is needed to preserve mass 
- Multigrid 
- Conjugate Gradient
- Fast Fourier Transform


### Diffusion of Densities
Implemented using the gauss siedel method 
![image](https://github.com/hiatus770/fluidSim/assets/77402029/0ec39728-da2f-4df9-9da3-b948d1268173)


https://github.com/hiatus770/fluidSim/assets/77402029/c9ff8abc-68fa-4538-bfaf-06b86bb0147d



## Advection
In terms of fluids advection is import as the densities and velocities will move according to the current velocities. This algorithm is O(n) where n is the amount of cells that it traverses. 

We use a semi lagrangian method to solve for the advection. We trace back in time and make the density at the current time the density at the previous time using interpolation. 
We go back in time by using negative velcoity at certain steps until we trace back to a starting point using the time difference given. 

## Projection

This is the last step, we want to get rid of divergence and curl of our velocity field
- Using hodge decomposiiton we can take any vector  field and split it by its mass conserving part and its gradient 
- The gradient has no curl and the mass conserving part has no divergence
- So we can subtract the divergence from the original field to get the mass conserving field of velocities
$$ \text{Mass Conserving Field} = \text{Initial Field} - \text{Gradient Field}$$ 
- The gradient is the direction of steepest descent of a height field 
- Derivative of X $` \text{Gx}[x, y] = \frac{1}{2} \cdot (x[x+1, y] - x[x-1, y]) / h `$ 
- Derivative of Y $` \text{Gy}[x, y] = \frac{1}{2} \cdot (x[x, y+1] - x[x, y-1]) / h `$
- What this is essentially doing is getting the divergence only field and removing it from the original vector field to leave us only with the curl

# Articles / Links
https://www.youtube.com/watch?v=x9xPX3WiK3E
https://www.youtube.com/watch?v=iKAVRgIrUOU&ab_channel=TenMinutePhysics
https://paveldogreat.github.io/WebGL-Fluid-Simulation/
http://graphics.cs.cmu.edu/nsp/course/15-464/Fall09/papers/StamFluidforGames.pdf



### Advection of Densities
Implemented using semi-lagrangian backtracing 
![image](https://github.com/hiatus770/fluidSim/assets/77402029/f1969843-e7bd-4296-baf9-dbb8646f9f7e)
