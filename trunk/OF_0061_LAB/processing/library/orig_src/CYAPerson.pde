class Rectangle
{
	float x, y, width, height;
}
class CYAPerson
{
	
	int id; 
	int age; 
 	PVector centroid;  
	PVector velocity; 
	Rectangle boundingRect;
	boolean dead;
	
	CYAPerson(){
		boundingRect = new Rectangle();
		centroid = new PVector();
		velocity = new PVector();
		dead = false;
	}
	
};