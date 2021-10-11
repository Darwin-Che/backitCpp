#include "client.h"

app_t::app_t() {
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
	super->mdt_sz = mdt_memctrl->tbend.idx;
	super->dirmap_sz = dirmap_memctrl->tbend.idx;
	super->fname_sz = fname_memctrl->tbend.idx;
	delete mdt_memctrl;
	delete dirmap_memctrl;
	delete fname_memctrl;
}

