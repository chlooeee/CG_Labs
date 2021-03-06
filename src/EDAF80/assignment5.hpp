#pragma once

#include <glm/glm.hpp>

class InputHandler;
class Window;


namespace edaf80
{
	//! \brief Wrapper class for Assignment 5
	class Assignment5 {
	public:
		//! \brief Default constructor.
		//!
		//! It will initialise various modules of bonobo and retrieve a
		//! window to draw to.
		Assignment5();

		//! \brief Default destructor.
		//!
		//! It will release the bonobo modules initialised by the
		//! constructor, as well as the window.
		~Assignment5();

		//! \brief Contains the logic of the assignment, along with the
		//! render loop.
		void run();

		bool testSphereSphere(glm::vec3 loc1, float radius1, glm::vec3 loc2, float radius2);


	private:
		InputHandler *inputHandler;
		Window       *window;
	};
}
