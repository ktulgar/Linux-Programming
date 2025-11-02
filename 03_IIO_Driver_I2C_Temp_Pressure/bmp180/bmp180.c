#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/iio/iio.h>
#include <linux/iio/sysfs.h>
#include <linux/of.h>
#include <linux/delay.h>  


#define DRIVER_NAME  "BMP180"


struct bmp180_calibration {

	short AC1;
	short AC2;
	short AC3;
	unsigned short AC4;
	unsigned short AC5;
	unsigned short AC6;
	short B1;
	short B2;
	short MB;
	short MC;
	short MD;
	
};


struct bmp180_data 
{
	struct i2c_client *client;
	struct mutex lock;
	struct bmp180_calibration calibration;
	
};



static const struct iio_chan_spec bmp180_channels[] = {
	{
		.type = IIO_TEMP,
		.info_mask_separate = BIT(IIO_CHAN_INFO_PROCESSED),
		
	},
	{
		.type = IIO_PRESSURE,
		.info_mask_separate = BIT(IIO_CHAN_INFO_PROCESSED),
		
	}
};


static const struct of_device_id bmp180_of_match[] = {
    { .compatible = "ktulgar,my_bmp180" },
    { /* sentinel */ }
};

static int bmp180_read_raw(struct iio_dev *indio_dev,const struct iio_chan_spec *chan,
 							int *val, int *val2, long mask)
{
	
 	struct bmp180_data *data = iio_priv(indio_dev);
 	int ret;
 	long UT;
	long T;
	long X1;
	long X2;
	long B5;
    	long p;
	long UP;
	unsigned char temBuffer[5];
	long B3;
	long B6;
	long X3;
	unsigned long B4;
	unsigned long B7;

     	switch(chan->type) 
    	{
   		case IIO_TEMP:
   			if(mask == IIO_CHAN_INFO_PROCESSED) 
   			{
   				
   			 	mutex_lock(&data->lock);
   			 	
    				temBuffer[0] = 0x2e;
				ret = i2c_smbus_write_i2c_block_data(data->client, 0xF4, 1 ,temBuffer);
				usleep_range(5000, 6000);
				ret = i2c_smbus_read_i2c_block_data(data->client, 0xf6, 2 ,temBuffer);
				UT = temBuffer[0] << 8 | temBuffer[1];

				X1 = (UT - data->calibration.AC6)*data->calibration.AC5/32768;
				X2 = data->calibration.MC * 2048 / (X1 + data->calibration.MD);
				B5 = X1 + X2;
				T = (B5 + 8)/16;
				*val = T;
				*val2 = 10;
				
 				mutex_unlock(&data->lock);
 				
 				return IIO_VAL_FRACTIONAL;
   			}	
   			
   		
   		case IIO_PRESSURE:
   			if(mask == IIO_CHAN_INFO_PROCESSED)
   			{
   				
   			
   				mutex_lock(&data->lock);
   				
   				temBuffer[0] = 0x34;
				ret = i2c_smbus_write_i2c_block_data(data->client, 0xF4, 1 ,temBuffer);
				usleep_range(5000, 6000);
				ret = i2c_smbus_read_i2c_block_data(data->client, 0xf6, 3,temBuffer);

				UP = ((temBuffer[0] << 16) |  (temBuffer[1] << 8) | (temBuffer[2])) >> 8;
   				
   				B6 =  B5 - 4000;
    				X1 = (data->calibration.B2 * (B6 * B6/4096))/2048;
   				X2 = data->calibration.AC2*B6/2048;
   				X3 = X1 + X2;
  				B3 = (((data->calibration.AC1*4+X3) << 0) + 2)/4;
    				X1 = data->calibration.AC3 * B6 / 8192;
   				X2 = (data->calibration.B1 * (B6 * B6 / 4096)) / 65536;
    				X3 = ((X1 + X2) + 2) / 4;
    				B4 = data->calibration.AC4 * (unsigned long)(X3 + 32768) / 32768;
   				B7 = ((unsigned long) UP - B3) * (50000 >> 0);
    				if(B7 < 0x80000000) p = (B7 * 2) / B4;
    				else p = (B7 / B4) * 2;
    				X1 = (p/256) * (p/256);
    				X1 = (X1 * 3038) / 65536;
    				X2 = (-7357 * p) / 65536;
    				p = p + (X1 + X2 + 3791)/16;
    				*val = (int) p;
    				
    				mutex_unlock(&data->lock);
    				
    				return IIO_VAL_INT;
   				
   			}
   		
    
    
    	}

 	return -EINVAL;
}


static const struct iio_info bmp180_info = {
	.read_raw = bmp180_read_raw,
};

static int bmp180_probe(struct i2c_client *client) 
{
	struct iio_dev *indio_dev;
	struct bmp180_data *data;
	int ret;
	
	indio_dev = devm_iio_device_alloc(&client->dev,sizeof(struct bmp180_data));
	data = iio_priv(indio_dev);
	data->client = client;
	mutex_init(&data->lock);
	
	indio_dev->dev.parent = &client->dev;
	indio_dev->name = DRIVER_NAME;

	indio_dev->info = &bmp180_info;
 	indio_dev->modes = INDIO_DIRECT_MODE;
 	indio_dev->channels = bmp180_channels;
 	indio_dev->num_channels = ARRAY_SIZE(bmp180_channels);
 	
 	
	unsigned char buffer[22]; 
	unsigned char STARTING_ADDRESS = 0xAA; 
 	ret = i2c_smbus_read_i2c_block_data(data->client, STARTING_ADDRESS, 22 ,buffer);

	data->calibration.AC1 = buffer[0] << 8  | buffer[1];
	data->calibration.AC2 = buffer[2] << 8  | buffer[3];
	data->calibration.AC3 = buffer[4] << 8  | buffer[5];
	data->calibration.AC4 = buffer[6] << 8  | buffer[7];
	data->calibration.AC5 = buffer[8] << 8  | buffer[9];
	data->calibration.AC6 = buffer[10] << 8 | buffer[11];
	data->calibration.B1  = buffer[12] << 8 | buffer[13];
	data->calibration.B2  = buffer[14] << 8 | buffer[15];
	data->calibration.MB  = buffer[16] << 8 | buffer[17];
	data->calibration.MC  = buffer[18] << 8 | buffer[19];
	data->calibration.MD  = buffer[20] << 8 | buffer[21];

	
 	ret = devm_iio_device_register(&client->dev, indio_dev);
 		
 	return 0;			
}

static struct i2c_driver bmp180_driver = {
 .driver = {
 .name = DRIVER_NAME,
 .of_match_table = bmp180_of_match,
 },
 .probe = bmp180_probe,

};
module_i2c_driver(bmp180_driver);
MODULE_DEVICE_TABLE(of, bmp180_of_match);
MODULE_AUTHOR("Kazım Tulgaroğlu");
MODULE_DESCRIPTION("A simple IIO temperature and pressure driver for educational purposes.");
MODULE_LICENSE("GPL");

