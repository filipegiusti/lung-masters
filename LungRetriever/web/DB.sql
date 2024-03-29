-- MySQL dump 10.11
--
-- Host: localhost    Database: lungRetriever
-- ------------------------------------------------------
-- Server version	5.0.75-0ubuntu10.2

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `CT_slice`
--

DROP TABLE IF EXISTS `CT_slice`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `CT_slice` (
  `id` int(11) NOT NULL auto_increment,
  `filename` varchar(255) default NULL,
  `leftLung` int(11) default NULL,
  `rightLung` int(11) default NULL,
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=8 DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `CT_slice`
--

LOCK TABLES `CT_slice` WRITE;
/*!40000 ALTER TABLE `CT_slice` DISABLE KEYS */;
INSERT INTO `CT_slice` VALUES (5,'svoduezmo',5,6),(6,'uymuiyy4m',7,8),(7,'d4wlaczv3',9,10);
/*!40000 ALTER TABLE `CT_slice` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `lungs`
--

DROP TABLE IF EXISTS `lungs`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `lungs` (
  `id` int(11) NOT NULL auto_increment,
  `energy` decimal(10,0) default NULL,
  `entropy` decimal(10,0) default NULL,
  `inverseDifferenceMoment` decimal(10,0) default NULL,
  `contrast` decimal(10,0) default NULL,
  `correlation` decimal(10,0) default NULL,
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=11 DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `lungs`
--

LOCK TABLES `lungs` WRITE;
/*!40000 ALTER TABLE `lungs` DISABLE KEYS */;
INSERT INTO `lungs` VALUES (5,'0','6','1','54','474897'),(6,'0','6','1','53','431737'),(7,'0','6','1','54','474897'),(8,'0','6','1','53','431737'),(9,'0','6','1','50','896253'),(10,'0','5','1','49','469575');
/*!40000 ALTER TABLE `lungs` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2009-06-12  3:44:51
