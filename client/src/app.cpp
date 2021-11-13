#include "client.h"

app_t::app_t(const char * rootdir) {
	load(rootdir);
	super_t * super = super_t::data;
	mdt_memctrl = new memctrl_t<mdt_t>;
	dirmap_memctrl = new memctrl_t<dirmap_t>;
	fname_memctrl = new memctrl_t<fname_t>;
	mdt_memctrl->frlst = super->mdt_fl;
	dirmap_memctrl->frlst = super->dirmap_fl;
	fname_memctrl->frlst = super->fname_fl;
	mdt_memctrl->tbend = super->mdt_sz;
	dirmap_memctrl->tbend = super->dirmap_sz;
	fname_memctrl->tbend = super->fname_sz;
}

app_t::~app_t() {
	super_t * super = super_t::data;
	super->mdt_fl = mdt_memctrl->frlst;
	super->dirmap_fl = dirmap_memctrl->frlst;
	super->fname_fl = fname_memctrl->frlst;
	super->mdt_sz = mdt_memctrl->tbend;
	super->dirmap_sz = dirmap_memctrl->tbend;
	super->fname_sz = fname_memctrl->tbend;
	delete mdt_memctrl;
	delete dirmap_memctrl;
	delete fname_memctrl;
	unload();
}

midx<mdt_t> app_t::add_track_unit(midx<mdt_t> parent, const char * str) {
	midx<mdt_t> mi;
	mdt_t * mp;
	midx<fname_t> fi;
	fname_t * fp;
	midx<dirmap_t> di;
	dirmap_t * dp;
	// verify the str is not already used
	mdt_memctrl->memalloc(mi);
	mp = ptr(mi);
	// create name
	fname_memctrl->memalloc(fi);
	mp->m_fname = fi;
	fp = ptr(mp->m_fname);
	strncpy(fp->fname, str, fname_t::MAXLEN-1);
	fp->fname[fname_t::MAXLEN-1] = 0;
	// set parent and insert head to parent's dirmap
	mp->m_parent = parent;
	dirmap_memctrl->memalloc(di);
	dp = ptr(di);
	dp->d_fname = fi;
	dp->d_mdtpos = mi;
	ptr(parent)->m_dirmap.insert_head(di);
	// other fields set to default
	mp->m_dirmap.init();
	// mdtp->m_sync_status = ;
	// mdtp->m_sync_time = ;
	return mi;
}

midx<mdt_t> app_t::add_track_file(midx<mdt_t> parent, const char * str) {
	// verify the file exists
	midx<mdt_t> ret = add_track_unit(parent, str);
	ptr(ret)->m_is_dir = false;
	return ret;
}

midx<mdt_t> app_t::add_track_dir(midx<mdt_t> parent, const char * str) {
	// verify the dir exists
	midx<mdt_t> ret = add_track_unit(parent, str);
	ptr(ret)->m_is_dir = true;
	return ret;
}
