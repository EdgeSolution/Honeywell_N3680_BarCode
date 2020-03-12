package honeywell.hedc;

import android.app.AlertDialog;
import android.content.Context;
import android.widget.ImageView;


public class GuiHelpers {
    protected Context context;

    public GuiHelpers(Context context) {
        this.context = context;
    }

    public String getVersionString(){
        return context.getString(R.string.hedc_rev);
    }

    public void ShowAlert(String title, String text, ImageView img) {
        AlertDialog.Builder Dlg = new AlertDialog.Builder(this.context);
        Dlg.setTitle(title);
        Dlg.setMessage(text);
        Dlg.setView(img);
        Dlg.setPositiveButton("OK", null);
        Dlg.show();
    }

    public void ShowAlert(String title, String text) {
        AlertDialog.Builder Dlg = new AlertDialog.Builder(this.context);
        Dlg.setTitle(title);
        Dlg.setMessage(text);
        Dlg.setPositiveButton("Close", null);
        Dlg.show();
    }

    public void ShowAlert(String title) {
        AlertDialog.Builder Dlg = new AlertDialog.Builder(this.context);
        Dlg.setTitle(title);
        Dlg.setMessage("");
        Dlg.setPositiveButton("Close", null);
        Dlg.show();
    }

    // FIXME need cleanup
    public String MakeHumanReadable(byte raw[]) {
        String s = "";
        for (int i = 0; i < raw.length; i++) {
            int rec = raw[i];
            if ((rec < 0x20)&&(rec >= 0)) {
//              s += String.format("<%s>",AsciiTab[rec]);
                s += String.format("<0x%02X>",rec);
            }
            else if (rec >= 0x7F) {
                s += String.format("<0x%02X>",rec);
            }
            else if (rec<0) {
                s += String.format("<0x%02X>",-rec);
            }
            else {
                s += String.format("%c", rec);
            }
        }
        return s;
    }

    public String MakeHumanReadableCmdResponse(byte raw[]) {
        String s, s1;
        s = "";
        for (int i = 0; i < raw.length; i++) {
            if (raw[i] == 0x06) {
                s1 = String.format("[ACK]");
            } else if (raw[i] == 0x05) {
                s1 = String.format("[NACK]");
            } else if (raw[i] == 0x15) {
                s1 = String.format("[ENQ]");
            } else {
                s1 = String.format("%c", raw[i]);
            }
            s += s1;
        }
        return s;
    }

    // Public: Creates a string from portions of byte array
    public String ComposeString(String prefix, final byte[] bytes, int start, int end) {
        String s=prefix;
        int len = end-start;
        if(len>0) {
            s += new String(bytes, start, len - 1);
        }
        return s;
    }

}
