package ascii_2nd_version;

import static ascii_2nd_version.Ascii_2nd_version.watchFolder;

/**
 *
 * @author AsifWasefi
 */
public class InputToOutput extends Thread
{
    private Thread t;
    private final String threadname;
    String folderToWatch = "/home/user/project/updatedVersion";
    private static final String fileToWatch = "input.txt";
    
    InputToOutput(String name)
    {
        threadname = name;
    }
    
    @Override
    public void run()
    {
        watchFolder(folderToWatch, fileToWatch);   
    }
    @Override
    public void start()
    {
        if (t==null){
            t = new Thread(this,threadname);
            t.start();
            //System.out.println("thread "+threadname+ " is gestart.");
        }
    }
}
