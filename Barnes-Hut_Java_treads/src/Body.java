import java.awt.Color;

public class Body {
    private double X, Y;       // position
    private double Vx, Vy;       // velocity
    private double fx, fy;       // force
    private double ax , ay;		//acceleration
    private double mass;         // mass
    private String name;
    private double minQuadlength;
    //private Color color;         // color

    // create and initialize a new Body
    public Body(double X, double Y, double Vx, double Vy, double mass, String name , double Mqlength) {
        this.X    = X;
        this.Y    = Y;
        this.Vx    = Vx;
        this.Vy    = Vy;
        this.mass  = mass;
        this.name = name;
        this.minQuadlength = Mqlength;
    }

    public void setMinQuadLength(double length){
    	this.minQuadlength = length;
    }
    public double getMinQuadLength() { return this.minQuadlength; }
    // update the velocity and position of the invoking Body
    // uses leapfrom method, as in Assignment 2
    public void update(double dt) {
    	
    	ax = fx/mass;
    	ay = fy/mass;
        Vx += dt * ax;
        Vy += dt * ay;
        X += dt * Vx;
        Y += dt * Vy;
    }

    // return the Euclidean distance bewteen the invoking Body and b
    public double distanceTo(Body b) {
        double dx = X - b.X;
        double dy = Y - b.Y;
        return Math.sqrt(dx*dx + dy*dy);
    }

    // reset the force of the invoking Body to 0
    public void resetForce() {
        fx = 0.0;
        fy = 0.0;
    }

    // compute the net force acting between the invoking body a and b, and
    // add to the net force acting on the invoking Body
    public void addForce(Body b) {
        double G = 6.67e-11;   // gravational constant
        Body a = this;
        double dx = b.X - a.X;
        double dy = b.Y - a.Y;
        double dist = Math.sqrt(dx*dx + dy*dy);
        double F = (G * a.mass * b.mass) / (dist*dist+1);
        a.fx += F * dx / dist;
        a.fy += F * dy / dist;
    }

    // convert to string representation formatted nicely
    public String toString() {
       // return String.format("%10.3E %10.3E %10.3E %10.3E %10.3E %s", X, Y, Vx, Vy, mass , name);
        return X+" "+ Y + " " + Vx+ " " + Vy+ " " + mass + " " + name ;
    }
    
    public void printb(){
    	System.out.println("name="+this.name+" x="+X+" y="+Y+" fy="+fy+" ay="+ay+" vy="+Vy);
    }
    public String mytostring() {
    	return "name="+this.name+" x="+X+" y="+Y+" fy="+fy+" ay="+ay+" vy="+Vy;
    }
    public void printc(){
    	if(name!=null)
    		System.out.println("name="+this.name+" x="+X+" y="+Y+" fy="+fy+" ay="+ay+" vy="+Vy);
    }
    //we can check if the body is in a certain quadrant
    public boolean in(Quad q) {
        return q.contains(this.X,this.Y);
    }
    void setname(String n){
    	this.name = n;
    }
    public Body add(Body b1 , Body b2) {
    	// m = m1 + m2
    	Body b = new Body(-1 , -1 , -1 , -1 , -1 , null , -1);
    	double m = b1.mass + b2.mass;
    	b.mass = m;
    	// x = (m1*x1 + m2*x2)/m
    	b.X = (b1.X*b1.mass + b2.X*b2.mass)/m;
    	// y = (y1*m1 + y2*m2)/m
    	b.Y = (b1.Y*b1.mass + b2.Y*b2.mass)/m;
    	return b;
    }

}
