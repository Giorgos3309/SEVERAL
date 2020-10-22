import java.awt.Color;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.PrintWriter;
import java.util.Scanner;
import java.util.concurrent.CyclicBarrier;

public class main {
	private final static int threads_num = 4;
    public static void main(String[] args) throws InterruptedException, FileNotFoundException {
    	if(args.length<2) {
        	System.out.println("Wrong programm parameters: <exe> <input file name> <rounds>");
        	System.exit(0);
        }
    	int rounds = Integer.parseInt(args[1]);
        Scanner inputStream = null;
        Scanner s = new Scanner(System.in); 
        
        try {
        	inputStream = new Scanner(new FileInputStream(args[0]));
        }catch(FileNotFoundException e){
        	System.out.println("input file was not found or could not be opened.");
        	System.exit(0);
        }
        int N = inputStream.nextInt();
        String Rstr = inputStream.next();
        double R =  Double.parseDouble(Rstr);
        
        
        Body[] bodies = new Body[N];               // array of N bodies
        
        for (int i = 0; i < N; i++) {
            String X   = inputStream.next();
            String Y   = inputStream.next();
            String Vx   = inputStream.next();
            String Vy   = inputStream.next();
            String mass = inputStream.next();
            String name = inputStream.next();
            bodies[i]   = new Body(Double.parseDouble(X), Double.parseDouble(Y), Double.parseDouble(Vx), Double.parseDouble(Vy), Double.parseDouble(mass), name , -1);
        }
        
        
        long startTime = System.currentTimeMillis();
        Quad q = new Quad(0 , 0 , R*2.0);
        BHTree tree = new BHTree(q); 
        //-------------------------------------------------------------------------------------------------------------------------------
        if(threads_num==1) {
        	CyclicBarrier barrier = new CyclicBarrier(1);
	        
	        BHthread thread1 = new BHthread(bodies , tree , 0 , N  , R , rounds , barrier );
	        
	        thread1.start();
	        
	        thread1.join();
	    }
        if(threads_num==2) {
	        CyclicBarrier barrier = new CyclicBarrier(2);
	        
	        BHthread thread1 = new BHthread(bodies , tree , 0 , N/2  , R , rounds , barrier );
	        BHthread thread2 = new BHthread(bodies , tree , N/2 , N  , R , rounds , barrier );
	        
	        thread1.start();
	        thread2.start();
	 
	        thread1.join();
	        thread2.join();
	 
        }else if(threads_num==4) {
        	CyclicBarrier barrier = new CyclicBarrier(4);
	        
	        BHthread thread1 = new BHthread(bodies , tree , 0 , N/4  , R , rounds , barrier );
	        BHthread thread2 = new BHthread(bodies , tree , N/4 , N/2  , R , rounds , barrier );
	        BHthread thread3 = new BHthread(bodies , tree , N/2 , 3*N/4  , R , rounds , barrier );
	        BHthread thread4 = new BHthread(bodies , tree , 3*N/4 , N  , R , rounds , barrier );
	        
	        thread1.start();
	        thread2.start();
	        thread3.start();
	        thread4.start();
	        
	        thread1.join();
	        thread2.join();
	        thread3.join();
	        thread4.join();
        }
        //--------------------------------------------------------------------------
        long estimatedTime = System.currentTimeMillis() - startTime;
        System.out.println(""+estimatedTime);
        PrintWriter writer = new PrintWriter("output.txt");
        for (int i = 0; i < N; i++) {
        	//System.out.println(bodies[i].toString());
        	writer.println(bodies[i].toString());
        }
        writer.close();
    }
}