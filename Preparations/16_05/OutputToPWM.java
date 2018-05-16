/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package ascii_2nd_version;

import static ascii_2nd_version.Ascii_2nd_version.inFile;
import java.io.DataInputStream;
import java.io.IOException;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardWatchEventKinds;
import java.nio.file.WatchEvent;
import java.nio.file.WatchKey;
import java.nio.file.WatchService;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 *
 * @author AsifWasefi
 */
public class OutputToPWM extends Thread {

    
    String folderToWatch = "/home/user/project/updatedVersion";
    private static final String fileToWatch = "output.txt";
    static String inputFile = "output.txt";
    private Thread t;
    private final String threadname;
    static private int doneLines=0;
    static private DataInputStream dis=null;
    
    OutputToPWM(String name)
    {
        threadname = name;
    }
    
    @Override
    public void run()
    {
        watchFolder(folderToWatch,fileToWatch);
    }
    
    @Override
    public void start()
    {
        if (t==null){
            t = new Thread(this,threadname);
            t.start();
            System.out.println("thread "+threadname+ " is gestart.");
        }
    }
    
    public static void watchFolder(String folder, String suffix) {
        
        try {
            Path path = Paths.get(folder);
            WatchService watchService = path.getFileSystem().newWatchService();
            path.register(watchService,StandardWatchEventKinds.ENTRY_MODIFY/*,StandardWatchEventKinds.ENTRY_CREATE,StandardWatchEventKinds.ENTRY_DELETE*/);
            
            convert();
            WatchKey key;
            while ((key = watchService.take()) != null) {
                for (WatchEvent<?> event : key.pollEvents()) {
                    Path changed = (Path) event.context();
                    if (changed.endsWith(suffix)) {
                        convert();
                    }
                }
                key.reset();
                
            }
        } catch (IOException | InterruptedException ex) {
            Logger.getLogger(Ascii_2nd_version.class.getName()).log(Level.SEVERE, null, ex);
        }


    }
    
    public static void convert()
    {
        dis = inFile(inputFile);
        try {
            if (dis !=null)
            { 
                String currentLine;
                int currentLineNr=0;
                while ( ( currentLine=dis.readLine()) != null){
                    if ((currentLineNr++) <= doneLines){
                        continue;
                    }
                    doneLines++;
                    int dump = Integer.valueOf(currentLine.trim());
                    //System.out.println("echo \""+dump+"\" | nc -u 192.168.0.4 3000");
                    System.out.println(dump);
                    
                }
            }
            dis.close();
        } catch (IOException ex) {
            Logger.getLogger(Ascii_2nd_version.class.getName()).log(Level.SEVERE, null, ex);
        }   
        }
    
}
