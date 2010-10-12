#include "CtBuffer.h"
#include "CtAccumulation.h"

using namespace lima;

bool CtBufferFrameCB::newFrameReady(const HwFrameInfoType& frame_info)
{
  DEB_MEMBER_FUNCT();
  DEB_PARAM() << DEB_VAR1(frame_info);

  Data fdata;
  CtBuffer::getDataFromHwFrameInfo(fdata,frame_info);
  if(m_ct_accumulation)
    return m_ct_accumulation->newFrameReady(fdata);
  else
    return m_ct->newFrameReady(fdata);
}

CtBuffer::CtBuffer(HwInterface *hw)
  : m_frame_cb(NULL),m_ct_accumulation(NULL)

{
  DEB_CONSTRUCTOR();

  if (!hw->getHwCtrlObj(m_hw_buffer))
    throw LIMA_CTL_EXC(Error, "Cannot get hardware buffer object");
}

CtBuffer::~CtBuffer()
{
  DEB_DESTRUCTOR();

  unregisterFrameCallback();
}

void CtBuffer::registerFrameCallback(CtControl *ct) 
{
  DEB_MEMBER_FUNCT();

  if (m_frame_cb == NULL) {
    m_frame_cb= new CtBufferFrameCB(ct);
    m_hw_buffer->registerFrameCallback(*m_frame_cb);
  }
}

void CtBuffer::unregisterFrameCallback() 
{
  DEB_MEMBER_FUNCT();
  
  if (m_frame_cb != NULL) {
    delete m_frame_cb;
    m_frame_cb= NULL;
  }
}

void CtBuffer::setPars(Parameters pars) 
{
  DEB_MEMBER_FUNCT();

  setMode(pars.mode);
  setNumber(pars.nbBuffers);
  setMaxMemory(pars.maxMemory);
}

void CtBuffer:: getPars(Parameters& pars) const
{
  DEB_MEMBER_FUNCT();

  pars= m_pars;

  DEB_RETURN() << DEB_VAR1(pars);
}

void CtBuffer:: setMode(BufferMode mode)
{
  DEB_MEMBER_FUNCT();
  DEB_PARAM() << DEB_VAR1(mode);

  m_pars.mode= mode;
}

void CtBuffer:: getMode(BufferMode& mode) const
{
  DEB_MEMBER_FUNCT();

  mode= m_pars.mode;

  DEB_RETURN() << DEB_VAR1(mode);
}

void CtBuffer:: setNumber(long nb_buffers)
{
  DEB_MEMBER_FUNCT();
  DEB_PARAM() << DEB_VAR1(nb_buffers);

  m_pars.nbBuffers= nb_buffers;
}

void CtBuffer:: getNumber(long& nb_buffers) const
{
  DEB_MEMBER_FUNCT();

  nb_buffers= m_pars.nbBuffers;

  DEB_RETURN() << DEB_VAR1(nb_buffers);
}

void CtBuffer:: setMaxMemory(short max_memory)
{
  DEB_MEMBER_FUNCT();
  DEB_PARAM() << DEB_VAR1(max_memory);

  if ((max_memory<1)||(max_memory>100))
    throw LIMA_CTL_EXC(InvalidValue, "Max memory usage between 1 and 100");

  m_pars.maxMemory= max_memory;
}
	
void CtBuffer::getMaxMemory(short& max_memory) const
{
  DEB_MEMBER_FUNCT();

  max_memory= m_pars.maxMemory;

  DEB_RETURN() << DEB_VAR1(max_memory);
}

void CtBuffer::getFrame(Data &aReturnData,int frameNumber)
{
  DEB_MEMBER_FUNCT();
  DEB_PARAM() << DEB_VAR1(frameNumber);

  if(m_ct_accumulation)
    m_ct_accumulation->getFrame(aReturnData,frameNumber);
  else
    {
      HwFrameInfo info;
      m_hw_buffer->getFrameInfo(frameNumber,info);
      getDataFromHwFrameInfo(aReturnData,info);
    }
  DEB_RETURN() << DEB_VAR1(aReturnData);
}

void CtBuffer::reset()
{
  DEB_MEMBER_FUNCT();

  m_pars.reset();
}

void CtBuffer::setup(CtControl *ct)
{
  DEB_MEMBER_FUNCT();

  CtAcquisition *acq;
  CtImage *img;
  AcqMode mode;
  FrameDim fdim;
  int acq_nframes, concat_nframes;

  acq= ct->acquisition();
  acq->getAcqMode(mode);
  acq->getAcqNbFrames(acq_nframes);

  img= ct->image();
  img->getHwImageDim(fdim);

  int hwNbBuffer = acq_nframes,nbuffers = acq_nframes;
  int max_nbuffers;
  m_hw_buffer->getMaxNbBuffers(max_nbuffers);
  m_ct_accumulation = NULL;
  switch (mode) {
  case Single:
    concat_nframes= 1;
    break;
  case Accumulation:
    m_hw_buffer->getNbConcatFrames(concat_nframes);
    concat_nframes= 1;
    m_ct_accumulation = ct->accumulation();
    hwNbBuffer = 16;
    nbuffers = max_nbuffers - hwNbBuffer;
    break;
  case Concatenation:
    acq->getConcatNbFrames(concat_nframes);
    break;
  }
  m_hw_buffer->setFrameDim(fdim);
  m_hw_buffer->setNbConcatFrames(concat_nframes);

  if (hwNbBuffer > max_nbuffers)
    hwNbBuffer = max_nbuffers;
  m_hw_buffer->setNbBuffers(hwNbBuffer);
  m_pars.nbBuffers = nbuffers;
  registerFrameCallback(ct);
  m_frame_cb->m_ct_accumulation = m_ct_accumulation;
}

void CtBuffer::getDataFromHwFrameInfo(Data &fdata,
				      const HwFrameInfoType& frame_info)
{
  DEB_STATIC_FUNCT();
  DEB_PARAM() << DEB_VAR1(frame_info);

  ImageType ftype;
  Size fsize;

  ftype= frame_info.frame_dim.getImageType();
  switch (ftype) {
  case Bpp8:
    fdata.type= Data::UINT8; break;
  case Bpp8S:
    fdata.type = Data::INT8; break;
  case Bpp10:
  case Bpp12:
  case Bpp14:
  case Bpp16:
    fdata.type= Data::UINT16; break;
  case Bpp16S:
    fdata.type = Data::INT16; break;
  case Bpp32:
    fdata.type= Data::UINT32; break;
  case Bpp32S:
    fdata.type = Data::INT32; break;
  default:
    THROW_CTL_ERROR(InvalidValue) << "Data type not yet managed" << DEB_VAR1(ftype);
  }

  fsize= frame_info.frame_dim.getSize();
  fdata.width= fsize.getWidth();
  fdata.height= fsize.getHeight();
  fdata.frameNumber= frame_info.acq_frame_nb;
  fdata.timestamp = frame_info.frame_timestamp;

  Buffer *fbuf = new Buffer();
  fbuf->data = frame_info.frame_ptr;
  if(frame_info.buffer_owner_ship == HwFrameInfoType::Transfer)
    fbuf->owner = Buffer::SHARED;
  else
    fbuf->owner = Buffer::MAPPED;	

  fdata.setBuffer(fbuf);
  fbuf->unref();
  
  DEB_RETURN() << DEB_VAR1(fdata);
}
// -----------------
// struct Parameters
// -----------------
CtBuffer::Parameters::Parameters()
{
  DEB_CONSTRUCTOR();

  reset();
}

void CtBuffer::Parameters::reset()
{
  DEB_MEMBER_FUNCT();

  mode= Linear;
  nbBuffers= 1;
  maxMemory= 75;

  DEB_TRACE() << *this;
}
