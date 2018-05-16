/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package ascii_2nd_version;

import java.io.BufferedInputStream;
import java.io.BufferedWriter;
import java.io.DataInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
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
public class Ascii_2nd_version {
    
    static String inputFile = "input.txt";
    static String outputFile = "output.txt";
    static int doneBytes=0;

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        InputToOutput thread1 = new InputToOutput("nodeMCU bytes naar Ascii");
        thread1.start();
        OutputToPWM thread2 = new OutputToPWM("Ascii to PWM");
        thread2.start();
        
        
    }
    
    public static DataInputStream inFile(String filename)
    {
        DataInputStream dis = null;
        try {
            
            dis = new DataInputStream(new BufferedInputStream(new FileInputStream(new File(filename))));
        } catch (FileNotFoundException ex) {
            Logger.getLogger(Ascii_2nd_version.class.getName()).log(Level.SEVERE, null, ex);
        }
        if (dis !=null) return dis;
        else return null;
        
    }
    
    /**
     * 
     * @param filename
     * @return 
     */
    public static PrintWriter outFile (String filename)
    {
        PrintWriter pw = null;
        try {  
            pw = new PrintWriter(new BufferedWriter(new FileWriter(new File(filename),true)));
        } catch (IOException ex) {
            Logger.getLogger(Ascii_2nd_version.class.getName()).log(Level.SEVERE, null, ex);
        }
        if (pw != null) return pw;
        else return null;
    }
    
    /**
     * watches a folder for changes
     * @param folder The folder to watch
     * @param suffix The file to watch
     * @param in the inputstream
     * @param out the outputstream
     */
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
    
    /**
     * 
     * @param dis
     * @param pw
     */
    public static void convert()
    {
        DataInputStream dis = inFile(inputFile);
        PrintWriter pw = outFile(outputFile);
        try {
            if (dis !=null && pw !=null)
            {
                int temp=dis.available()-doneBytes;
                int dump = dis.skipBytes(doneBytes);
                for ( int i=0 ; i<temp ; i++){
                    doneBytes++;
                    byte toWriteByte = dis.readByte();
                    if (toWriteByte != 0 && toWriteByte !=10 && toWriteByte != 13 && toWriteByte !=32){
                        int toWriteByteAscii = toWriteByte;
                        pw.write(String.valueOf(toWriteByteAscii));pw.write("\n");
                    } 
                }
            }
            dis.close();
            pw.close();
        } catch (IOException ex) {
            Logger.getLogger(Ascii_2nd_version.class.getName()).log(Level.SEVERE, null, ex);
        }   
        }
        
    }
    

