using System;

namespace OpenHome.Media
{
    public static class BigEndianConverter
    {
        public static int BigEndianToInt32(byte[] aValue, int aStartIndex)
        {
            if (System.BitConverter.IsLittleEndian)
            {
                byte[] value = Reverse(aValue, aStartIndex, 4);
                return System.BitConverter.ToInt32(value, 0);
            }
            else
            {
                return System.BitConverter.ToInt32(aValue, aStartIndex);
            }
        }
    
        public static uint BigEndianToUint32(byte[] aValue, int aStartIndex)
        {
            if (System.BitConverter.IsLittleEndian)
            {
                byte[] value = Reverse(aValue, aStartIndex, 4);
                return System.BitConverter.ToUInt32(value, 0);
            }
            else
            {
                return System.BitConverter.ToUInt32(aValue, aStartIndex);
            }
        }
    
        public static byte[] Int32ToBigEndian(int aValue)
        {
            byte[] b = System.BitConverter.GetBytes(aValue);
    
            if(System.BitConverter.IsLittleEndian)
            {
                b = Reverse(b, 0, b.Length);
            }
    
            return b;
        }
    
        public static byte[] Uint32ToBigEndian(uint aValue)
        {
            byte[] b =  System.BitConverter.GetBytes(aValue);
    
            if(System.BitConverter.IsLittleEndian)
            {
                b = Reverse(b, 0, b.Length);
            }
    
            return b;
        }
    
        private static byte[] Reverse(byte[] aData, int aOffset, int aLength)
        {
            byte[] reverse = new byte[aLength];
            for (int i = 0; i < aLength; i++)
            {
                reverse[i] = aData[aOffset + aLength - 1 - i];
            }
            return reverse;
        }
    }
}
