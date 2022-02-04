package main

import (
	"bufio"
	"bytes"
	"compress/gzip"
	"errors"
	"flag"
	"fmt"
	"io/ioutil"
	"os"
	"path/filepath"
	"regexp"
	"strings"

	"github.com/tdewolff/minify/v2"
	"github.com/tdewolff/minify/v2/css"
	"github.com/tdewolff/minify/v2/html"
	"github.com/tdewolff/minify/v2/js"
	"github.com/tdewolff/minify/v2/json"
	"github.com/tdewolff/minify/v2/svg"
	"github.com/tdewolff/minify/v2/xml"
)

var filetypeMime = map[string]string{
	"css":  "text/css",
	"htm":  "text/html",
	"html": "text/html",
	"js":   "application/javascript",
	"json": "application/json",
	"svg":  "image/svg+xml",
	"xml":  "text/xml",
}

func writeBufToFile(fname string, buf *bytes.Buffer) error {
	os.Remove(fname)

	fo, err := os.OpenFile(fname, os.O_WRONLY|os.O_CREATE, 0666)
	if err != nil {
		return err
	}
	defer fo.Close()

	fow := bufio.NewWriter(fo)

	_, err = fow.Write(buf.Bytes())
	if err != nil {
		return err
	}

	if err := fow.Flush(); err != nil {
		return err
	}

	if err := fo.Close(); err != nil {
		return err
	}
	return nil
}

func writeGzBufToFile(fname string, buf *bytes.Buffer) error {
	os.Remove(fname)

	fi, err := os.OpenFile(fname, os.O_WRONLY|os.O_CREATE, 0666)
	if err != nil {
		return err
	}
	defer fi.Close()

	gf := gzip.NewWriter(fi)
	defer gf.Close()

	gfw := bufio.NewWriter(gf)
	defer gfw.Flush()

	_, err = gfw.Write(buf.Bytes())
	if err != nil {
		return err
	}

	return nil
}

func writeBufToArrayFile(fname string, buf *bytes.Buffer) error {
	os.Remove(fname)

	fo, err := os.OpenFile(fname, os.O_WRONLY|os.O_CREATE, 0666)
	if err != nil {
		return err
	}
	fow := bufio.NewWriter(fo)

	//File: main.js.gz, Size: 2667
	// #define main_js_gz_len 2667
	// const uint8_t main_js_gz[] PROGMEM = {

	blen := buf.Len()
	idx := 0
	vname := filepath.Base(fname)[:strings.Index(filepath.Base(fname), ".h")]

	fmt.Fprintf(fow, "#define %s_len %d\nconst unsigned char %s[] PROGMEM = {\n", vname, blen, vname)
	for i, b := range buf.Bytes() {
		fmt.Fprintf(fow, " 0x%02X", b)
		if i < (blen - 1) {
			fmt.Fprintf(fow, ",")
		}
		if (i % 16) == 15 {
			fmt.Fprintf(fow, "\n")
		}
		idx = i
	}

	fmt.Printf("buffer len: %d, idx: %d\r\n", blen, idx)

	fmt.Fprintf(fow, "\n};\n\n")

	if err := fow.Flush(); err != nil {
		return err
	}

	if err := fo.Close(); err != nil {
		return err
	}
	return nil
}

func minifyFile(fsrc string, buf *bytes.Buffer) error {
	m := minify.New()
	m.AddFunc("text/css", css.Minify)
	m.AddFunc("text/html", html.Minify)
	m.AddFunc("image/svg+xml", svg.Minify)
	m.AddFuncRegexp(regexp.MustCompile("^(application|text)/(x-)?(java|ecma)script$"), js.Minify)
	m.AddFuncRegexp(regexp.MustCompile("[/+]json$"), json.Minify)
	m.AddFuncRegexp(regexp.MustCompile("[/+]xml$"), xml.Minify)
	m.Add("text/html", &html.Minifier{
		KeepDefaultAttrVals: true,
		KeepDocumentTags: true,
	})
	m.AddRegexp(regexp.MustCompile("^(application|text)/(x-)?(java|ecma)script$"), &js.Minifier{
		KeepVarNames: true,
	})
	fmt.Println("file input: " + fsrc)
	ext := filepath.Ext(fsrc)
	if 0 < len(ext) {
		ext = ext[1:]
	}

	fmt.Println("file extension: " + ext)

	mf, err := os.Open(fsrc)
	defer mf.Close()

	if err != nil {
		return err
	}

	if strings.Contains(fsrc, ".min.") {
		fmt.Println("file is already minimized, copying.")
		bufio.NewWriter(buf).ReadFrom(mf)
		return nil
	}

	if strings.Compare("png", ext) == 0 {
		bufio.NewWriter(buf).ReadFrom(mf)
		return nil
	}

	if _, ok := filetypeMime[ext]; !ok {
		return errors.New("cannot find file extension: " + ext)
	}

	mfr := bufio.NewReader(mf)
	mfbw := bufio.NewWriter(buf)

	// fmt.Println(os.Stdin)
	if err := m.Minify(filetypeMime[ext], mfbw, mfr); err != nil {
		return err
	}

	if err := mfbw.Flush(); err != nil {
		return err
	}

	if err := mf.Close(); err != nil {
		return err
	}

	return nil
}

func gzipWrite(bufin *bytes.Buffer, bufout *bytes.Buffer) error {
	gf := gzip.NewWriter(bufout)

	_, err := gf.Write(bufin.Bytes())
	if err != nil {
		return err
	}

	if err := gf.Close(); err != nil {
		return err
	}

	return nil
}

func minifyGZFile(fsrc, fout, tmpout string) error {
	var mBuf bytes.Buffer
	var gfBuf bytes.Buffer

	if err := minifyFile(fsrc, &mBuf); err != nil {
		return err
	}

	if err := writeBufToFile(tmpout + ".min", &mBuf); err != nil {
		return err
	}

	if err := gzipWrite(&mBuf, &gfBuf); err != nil {
		return err
	}

	// if err := writeBufToArrayFile(fout + ".h", &mBuf); err != nil {
	// 	return err
	// }

	if err := writeGzBufToFile(tmpout + ".gz", &gfBuf); err != nil {
		return err
	}

	if err := writeBufToArrayFile(fout + "_gz.h", &gfBuf); err != nil {
		return err
	}
	mBuf.Reset()
	gfBuf.Reset()

	return nil
}

func main() {
	src := ""
	oDir := ""
	gzDir := ""

	flag.StringVar(&src, "d", "../html", "Input filenames")
	flag.StringVar(&oDir, "o", "../esp8266-servo-async/src/html_h", "Out Directory")
	flag.StringVar(&gzDir, "g", "./gz", "Gzip Out Directory")
	flag.Parse()

	files, err := ioutil.ReadDir(src)
	if err != nil {
		panic(err)
	}

	if _, err := os.Stat(gzDir); os.IsNotExist(err) {
		os.Mkdir(gzDir, 0755);
    }

	for _, file := range files {
		if !file.IsDir() {
			fname := file.Name()
			fpath := src + "/" + fname
			fout := oDir + "/" + strings.Replace(fname, ".", "_", -1) + "_min"
			gzout := gzDir + "/" + fname
			fmt.Println(fname + ">" + fout)
			if err := minifyGZFile(fpath, fout, gzout); err != nil {
				fmt.Println("Error: " + err.Error())
				continue
			}
		}
	}
}
