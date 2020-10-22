import java.util.concurrent.BrokenBarrierException;
import java.util.concurrent.CyclicBarrier;

public class BHthread extends Thread {
	private final int dt = 1;
	private int start , end;
	private	int rounds;
	private Body[] bodies;
	private Quad q;
	private double R;
	private BHTree tree;
	CyclicBarrier barrier;
	
	
	public BHthread(Body []b , BHTree tree , int start , int end ,  double R , int rounds , CyclicBarrier br ){
		this.tree = tree;
        this.bodies = b;
        this.start = start;
        this.end = end;
        this.R = R;
        this.rounds = rounds;
        this.barrier = br;
	}
	
	public void run() {
		for(int j=0; j<rounds; ++j) {     
        	for (int i = start; i < end; i++) {
	        	bodies[i].resetForce();
	        }
        	try {
				barrier.await();
			} catch (InterruptedException | BrokenBarrierException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
        	synchronized(tree) {
		        for (int i = start; i < end; i++) {
		        		tree.insert(bodies[i]); 
		        }
        	}

	        try {
				barrier.await();
			} catch (InterruptedException | BrokenBarrierException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
	      // synchronized(bodies) {
		        for (int i = start; i < end; i++) {
		        	tree.updateForce(bodies[i]);
		        }
	        //}
	        try {
				barrier.await();
			} catch (InterruptedException | BrokenBarrierException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
	        for (int i = start; i < end; i++) {
	        	bodies[i].update(i);
	        }
	        q = new Quad(0 , 0 , R*2.0);
	        tree = new BHTree(q);
	        try {
				barrier.await();
			} catch (InterruptedException | BrokenBarrierException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
        }
		
	}

}
