/****************************************************************************
Copyright (c) 2010-2012 cocos2d-x.org
Copyright (c) 2013-2016 Chukong Technologies Inc.
Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.

http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
 ****************************************************************************/
package org.cocos2dx.lib;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.Typeface;
import android.text.BoringLayout;
import android.text.Layout;
// CROWDSTAR_COCOSPATCH_BEGIN(BitmapShadowTags)
import android.text.SpannableString;
// CROWDSTAR_COCOSPATCH_END
import android.text.StaticLayout;
import android.text.TextPaint;
import android.text.TextUtils;
// CROWDSTAR_COCOSPATCH_BEGIN(BitmapShadowTags)
import android.text.style.StrikethroughSpan;
// CROWDSTAR_COCOSPATCH_END
import android.util.Log;
// CROWDSTAR_COCOSPATCH_BEGIN(BitmapShadowTags)
import android.text.style.ForegroundColorSpan;
import android.text.*;
import android.graphics.Color;
import android.text.style.RelativeSizeSpan;
import android.text.style.TypefaceSpan;
import android.text.style.StyleSpan;
import android.text.style.UnderlineSpan;
// CROWDSTAR_COCOSPATCH_END
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
// CROWDSTAR_COCOSPATCH_BEGIN(BitmapShadowTags)
import java.util.ArrayList;
// CROWDSTAR_COCOSPATCH_END

public final class Cocos2dxBitmap {
    // ===========================================================
    // Constants
    // ===========================================================

    /* The values are the same as cocos2dx/platform/CCImage.h. */
    private static final int HORIZONTAL_ALIGN_LEFT = 1;
    private static final int HORIZONTAL_ALIGN_RIGHT = 2;
    private static final int HORIZONTAL_ALIGN_CENTER = 3;
    private static final int VERTICAL_ALIGN_TOP = 1;
    private static final int VERTICAL_ALIGN_BOTTOM = 2;
    private static final int VERTICAL_ALIGN_CENTER = 3;

// CROWDSTAR_COCOSPATCH_BEGIN(BitmapShadowTags)
    private static ArrayList tags = new ArrayList();
// CROWDSTAR_COCOSPATCH_END

    // ===========================================================
    // Fields
    // ===========================================================

    private static Context sContext;

    // ===========================================================
    // Getter & Setter
    // ===========================================================

    public static void setContext(final Context context) {
        Cocos2dxBitmap.sContext = context;
    }

    // ===========================================================
    // Methods for/from SuperClass/Interfaces
    // ===========================================================

    // ===========================================================
    // Methods
    // ===========================================================

    private static native void nativeInitBitmapDC(final int width,
            final int height, final byte[] pixels);

    //http://egoco.de/post/19077604048/calculating-the-height-of-text-in-android
    public static int getTextHeight(String text, int maxWidth, float textSize, Typeface typeface) {
        TextPaint paint = new TextPaint(Paint.ANTI_ALIAS_FLAG | Paint.SUBPIXEL_TEXT_FLAG);
        paint.setTextSize(textSize);
        paint.setTypeface(typeface);

        int lineCount = 0;

        int index = 0;
        int length = text.length();

        while(index < length) {
            final int charsToAdvance = paint.breakText(text, index, length, true, maxWidth, null);
            if(charsToAdvance == 0) {
                index++;
            }
            else {
                index += charsToAdvance;
                lineCount++;
            }
        }

        float actualHeight = (Math.abs(paint.ascent()) + Math.abs(paint.descent()));

        return (int)Math.floor(lineCount * actualHeight);
    }

    public static Typeface calculateShrinkTypeFace(String text, int width, int height, Layout.Alignment hAlignment, float textSize, TextPaint paint, boolean enableWrap)
    {
        if (width == 0 || height == 0) {
            return  paint.getTypeface();
        }
        float actualWidth = width + 1;
        float actualHeight = height + 1;
        float fontSize = textSize + 1;

        if (!enableWrap) {
            while (actualWidth > width || actualHeight > height) {
                fontSize = fontSize - 1;

                actualWidth = (int)Math.ceil( StaticLayout.getDesiredWidth(text, paint));
                actualHeight = getTextHeight(text, (int)actualWidth, fontSize, paint.getTypeface());

                paint.setTextSize(fontSize);
                if (fontSize <= 0) {
                    paint.setTextSize(textSize);
                    break;
                }
            }
        } else {
            while (actualHeight > height || actualWidth > width) {
                fontSize = fontSize - 1;

                Layout layout = new StaticLayout(text, paint, (int) width, hAlignment,1.0f,0.0f,false);
                actualWidth = layout.getWidth();
                actualHeight = layout.getLineTop(layout.getLineCount());

                paint.setTextSize(fontSize);

                if (fontSize <= 0) {
                    paint.setTextSize(textSize);
                    break;
                }
            }

        }
        return paint.getTypeface();
    }

    public static boolean createTextBitmapShadowStroke(byte[] bytes,  final String fontName, int fontSize,
                                                    int fontTintR, int fontTintG, int fontTintB, int fontTintA,
                                                    int alignment, int width, int height, 
                                                    boolean shadow, float shadowDX, float shadowDY, float shadowBlur, float shadowOpacity, 
                                                    boolean stroke, int strokeR, int strokeG, int strokeB, int strokeA, float strokeSize, boolean enableWrap, int overflow) {
        String string;
        if (bytes == null || bytes.length == 0) {
          return false;
        } else {
          string = new String(bytes);
        }

        Layout.Alignment hAlignment = Layout.Alignment.ALIGN_NORMAL;
        int horizontalAlignment = alignment & 0x0F;
        switch (horizontalAlignment) {
            case HORIZONTAL_ALIGN_CENTER:
                hAlignment = Layout.Alignment.ALIGN_CENTER;
                break;
            case HORIZONTAL_ALIGN_RIGHT:
                hAlignment = Layout.Alignment.ALIGN_OPPOSITE;
                break;
            case HORIZONTAL_ALIGN_LEFT:
                break;
            default:
                break;
        }

        TextPaint paint = Cocos2dxBitmap.newPaint(fontName, fontSize);

        if (stroke) {
            paint.setStyle(TextPaint.Style.STROKE);
            paint.setStrokeWidth(strokeSize);
        }

        int maxWidth = width;

        if (maxWidth <= 0) {
            maxWidth = (int)Math.ceil( StaticLayout.getDesiredWidth(string, paint));
        }

// CROWDSTAR_COCOSPATCH_BEGIN(BitmapShadowTags)
        if(tags.isEmpty())
        {
            tags.add("<red_1>");
            tags.add("<strike_1>");
            tags.add("<green_1>");
            tags.add("<blue_1>");
            tags.add("<gray_1>");
            tags.add("<bold>");
            tags.add("<BOLD>");
            tags.add("<gold_1>");
            tags.add("<blue_1>");
            tags.add("<black_1>");
            tags.add("<white_1>");
            tags.add("<underline>");
        }

        ArrayList startIndex = new ArrayList();
        ArrayList rangeIndex = new ArrayList();
        ArrayList tagList = new ArrayList();

        String endTag = "</>";
        int prevIndex = 0;
        boolean search = true;
        boolean hasTag = false;
        int totalTagLength = 0;
        while(search)
        {
            int tagLength = 0;
            int tagStart = -1;
            int tagEnd = -1;
            String tag = "";
            for (int j = 0 ; j < tags.size(); j++)
            {
                String newTag = (String)tags.get(j);
                int newTagStart = string.indexOf(newTag, prevIndex);
                int newTagEnd = string.indexOf(endTag, prevIndex);

                if(newTagStart == -1 || newTagEnd == -1)
                {
                    continue;
                }

                if( newTagStart < tagStart || tagStart == -1)
                {
                    tagStart = newTagStart;
                    tag = newTag;
                    tagEnd = newTagEnd;
                }
            }

            if(tagStart == -1 || tagEnd == -1 )
            {
                search = false;
            }
            else
            {
                tagStart -= totalTagLength;
                tagEnd -= totalTagLength;
                tagLength = tag.length();
                totalTagLength += tagLength + endTag.length();
                startIndex.add(tagStart);
                rangeIndex.add(tagEnd - (tagStart + tagLength));
                tagList.add(tag);
                hasTag = true;
                prevIndex += tagEnd + 1;
            }
        }

        String newString = string;

        if(hasTag)
        {
            for (int i = 0 ; i < tagList.size(); i++)
            {
                String tag = (String)tagList.get(i);
                newString = newString.replaceFirst(tag, "");
                newString = newString.replaceFirst("</>", "");
            }
        }

        SpannableString attribString = new SpannableString(newString);

        for (int i = 0 ; i < startIndex.size(); i++)
        {
            String tag = (String)tagList.get(i);

            int tagStart = (int) startIndex.get(i);
            int tagEnd = tagStart + (int) rangeIndex.get(i);

            if(tagEnd > attribString.length())
                continue;

            if(tag == "<red_1>") {
                attribString.setSpan(new ForegroundColorSpan(Color.rgb(216, 45, 60)), tagStart, tagEnd, Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
            }
            else if(tag.equalsIgnoreCase("<strike_1>"))
            {
                attribString.setSpan(new ForegroundColorSpan(Color.GRAY), tagStart, tagEnd, Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
                attribString.setSpan(new StrikethroughSpan(), tagStart, tagEnd, Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
            }
            else if(tag.equalsIgnoreCase("<green_1>")) {
                attribString.setSpan(new ForegroundColorSpan(Color.rgb(79,186,106)), tagStart, tagEnd, Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
            }
            else if(tag.equalsIgnoreCase("<blue_1>")) {
                attribString.setSpan(new ForegroundColorSpan(Color.rgb(58,155,252)), tagStart, tagEnd, Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
            }
            else if(tag.equalsIgnoreCase("<gray_1>")) {
                attribString.setSpan(new ForegroundColorSpan(Color.GRAY), tagStart, tagEnd, Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
            }
            else if(tag.equalsIgnoreCase("<bold>")) {
                TypefaceSpan typefaceSpan = new TypefaceSpan("Covet-Bold.ttf");
                attribString.setSpan(new StyleSpan(Typeface.BOLD), tagStart, tagEnd, Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
                attribString.setSpan(typefaceSpan, tagStart, tagEnd, Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
            }
            else if(tag.equalsIgnoreCase("<gold_1>")) {
                attribString.setSpan(new ForegroundColorSpan(Color.rgb(203,170,67)), tagStart, tagEnd, Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
            }
            else if(tag.equalsIgnoreCase("<blue_1>")) {
                attribString.setSpan(new ForegroundColorSpan(Color.rgb(58,155,252)), tagStart, tagEnd, Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
            }
            else if(tag.equalsIgnoreCase("<black_1>")) {
                attribString.setSpan(new ForegroundColorSpan(Color.BLACK), tagStart, tagEnd, Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
            }
            else if(tag.equalsIgnoreCase("<white_1>")) {
                attribString.setSpan(new ForegroundColorSpan(Color.rgb(256,256,256)), tagStart, tagEnd, Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
            }
            else if(tag.equalsIgnoreCase("<underline>")) {
                attribString.setSpan(new UnderlineSpan(), tagStart, tagEnd, Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
            }
        }
// CROWDSTAR_COCOSPATCH_END

        Layout layout = null;
        int layoutWidth = 0;
        int layoutHeight = 0;


        if (overflow == 1 && !enableWrap){
            int widthBoundary = (int)Math.ceil( StaticLayout.getDesiredWidth(string, paint));
// CROWDSTAR_COCOSPATCH_BEGIN(BitmapShadowTags)
// Changed string to attribString
            layout = new StaticLayout(attribString, paint, widthBoundary , hAlignment,1.0f,0.0f,false);
// CROWDSTAR_COCOSPATCH_END
        }else {
            if (overflow == 2) {
                calculateShrinkTypeFace(string, width, height, hAlignment, fontSize, paint, enableWrap);
            }
// CROWDSTAR_COCOSPATCH_BEGIN(BitmapShadowTags)
// Changed string to attribString
            layout = new StaticLayout(attribString, paint, maxWidth , hAlignment,1.0f,0.0f,false);
// CROWDSTAR_COCOSPATCH_END
        }

        layoutWidth = layout.getWidth();
        layoutHeight = layout.getLineTop(layout.getLineCount());

        int bitmapWidth = Math.max(layoutWidth, width);
        int bitmapHeight = layoutHeight;
        if (height > 0) {
            bitmapHeight = height;
        }

        if (overflow == 1 && !enableWrap) {
            if (width > 0) {
                bitmapWidth = width;
            }
        }

        if (bitmapWidth == 0 || bitmapHeight == 0) {
            return false;
        }

        int offsetX = 0;
        if (horizontalAlignment == HORIZONTAL_ALIGN_CENTER) {
            offsetX = (bitmapWidth - layoutWidth) / 2;
        }
        else if (horizontalAlignment == HORIZONTAL_ALIGN_RIGHT) {
            offsetX = bitmapWidth - layoutWidth;
        }

        int offsetY = 0;
        int verticalAlignment   = (alignment >> 4) & 0x0F;
        switch (verticalAlignment)
        {
            case VERTICAL_ALIGN_CENTER:
                offsetY = (bitmapHeight - layoutHeight) / 2;
                break;
            case VERTICAL_ALIGN_BOTTOM:
                offsetY = bitmapHeight - layoutHeight;
                break;
        }



        Bitmap bitmap = Bitmap.createBitmap(bitmapWidth, bitmapHeight, Bitmap.Config.ARGB_8888);
        Canvas canvas = new Canvas(bitmap);
        canvas.translate(offsetX, offsetY);
        if ( stroke )
        {
            paint.setARGB(strokeA, strokeR, strokeG, strokeB);
            layout.draw(canvas);
        }
        paint.setStyle(TextPaint.Style.FILL);
        paint.setARGB(fontTintA, fontTintR, fontTintG, fontTintB);
        layout.draw(canvas);

        Cocos2dxBitmap.initNativeObject(bitmap);
        return true;
    }

    private static TextPaint newPaint(final String fontName, final int fontSize) {
        final TextPaint paint = new TextPaint();
        paint.setTextSize(fontSize);
        paint.setAntiAlias(true);

        // Set type face for paint, now it support .ttf file.
        if (fontName.endsWith(".ttf")) {
            try {
                final Typeface typeFace = Cocos2dxTypefaces.get(
                        Cocos2dxBitmap.sContext, fontName);
                paint.setTypeface(typeFace);
            } catch (final Exception e) {
                Log.e("Cocos2dxBitmap", "error to create ttf type face: "
                        + fontName);

                // The file may not find, use system font.
                paint.setTypeface(Typeface.create(fontName, Typeface.NORMAL));
            }
        } else {
            paint.setTypeface(Typeface.create(fontName, Typeface.NORMAL));
        }

        return paint;
    }

    private static void initNativeObject(final Bitmap bitmap) {
        final byte[] pixels = Cocos2dxBitmap.getPixels(bitmap);
        if (pixels == null) {
            return;
        }

        Cocos2dxBitmap.nativeInitBitmapDC(bitmap.getWidth(),
                bitmap.getHeight(), pixels);
    }

    private static byte[] getPixels(final Bitmap bitmap) {
        if (bitmap != null) {
            final byte[] pixels = new byte[bitmap.getWidth()
                    * bitmap.getHeight() * 4];
            final ByteBuffer buf = ByteBuffer.wrap(pixels);
            buf.order(ByteOrder.nativeOrder());
            bitmap.copyPixelsToBuffer(buf);
            return pixels;
        }

        return null;
    }

    public static int getFontSizeAccordingHeight(int height) {
        TextPaint paint = new TextPaint();
        Rect bounds = new Rect();

        paint.setTypeface(Typeface.DEFAULT);
        int text_size = 1;
        boolean found_desired_size = false;

        while (!found_desired_size) {
            paint.setTextSize(text_size);
            String text = "SghMNy";
            paint.getTextBounds(text, 0, text.length(), bounds);

            text_size++;

            if (height - bounds.height() <= 2) {
                found_desired_size = true;
            }
        }
        return text_size;
    }

    private static String getStringWithEllipsis(String string, float width,
                                                float fontSize) {
        if (TextUtils.isEmpty(string)) {
            return "";
        }

        TextPaint paint = new TextPaint();
        paint.setTypeface(Typeface.DEFAULT);
        paint.setTextSize(fontSize);

        return TextUtils.ellipsize(string, paint, width,
                TextUtils.TruncateAt.END).toString();
    }
}
