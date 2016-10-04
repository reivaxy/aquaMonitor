-- phpMyAdmin SQL Dump
-- version 4.4.15.7
-- http://www.phpmyadmin.net
--
-- Généré le :  Dim 02 Octobre 2016 à 23:07
-- Version du serveur :  5.5.50-0+deb7u2-log
-- Version de PHP :  5.4.45-0+deb7u4

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
--

-- --------------------------------------------------------

--
-- Structure de la table `aquanetmodule`
--

CREATE TABLE IF NOT EXISTS `aquanetmodule` (
  `moduleid` int(11) NOT NULL,
  `userid` int(11) NOT NULL,
  `name` tinytext NOT NULL,
  `description` varchar(1000) NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Structure de la table `aquanetstat`
--

CREATE TABLE IF NOT EXISTS `aquanetstat` (
  `statid` int(11) NOT NULL,
  `userid` int(11) NOT NULL,
  `moduleid` int(11) NOT NULL,
  `dbdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `datadate` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',
  `power` smallint(6) NOT NULL COMMENT 'smallint in case someday I log the value',
  `poweralert` tinyint(1) NOT NULL,
  `waterlevel` smallint(6) NOT NULL COMMENT 'smallint in case some day I log a value',
  `waterlevelalert` tinyint(1) NOT NULL,
  `lightlevel` smallint(6) NOT NULL,
  `lightlevelalert` tinyint(1) NOT NULL,
  `temperature` decimal(4,0) NOT NULL,
  `temperaturealert` tinyint(1) NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Structure de la table `aquanetuser`
--

CREATE TABLE IF NOT EXISTS `aquanetuser` (
  `userid` int(11) NOT NULL,
  `apikey` char(36) NOT NULL,
  `name` tinytext NOT NULL,
  `firstname` tinytext NOT NULL,
  `email` tinytext NOT NULL,
  `lastUpdate` datetime NOT NULL,
  `enabled` tinyint(1) NOT NULL DEFAULT '1'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Structure de la table `logs`
--

CREATE TABLE IF NOT EXISTS `logs` (
  `id` int(11) NOT NULL,
  `log` text NOT NULL,
  `date` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Index pour les tables exportées
--

--
-- Index pour la table `aquanetmodule`
--
ALTER TABLE `aquanetmodule`
  ADD PRIMARY KEY (`moduleid`);

--
-- Index pour la table `aquanetstat`
--
ALTER TABLE `aquanetstat`
  ADD UNIQUE KEY `statid` (`statid`),
  ADD KEY `userid` (`userid`),
  ADD KEY `moduleid` (`moduleid`);

--
-- Index pour la table `aquanetuser`
--
ALTER TABLE `aquanetuser`
  ADD PRIMARY KEY (`userid`),
  ADD UNIQUE KEY `apikey` (`apikey`);

--
-- Index pour la table `logs`
--
ALTER TABLE `logs`
  ADD PRIMARY KEY (`id`);

--
-- AUTO_INCREMENT pour les tables exportées
--

--
-- AUTO_INCREMENT pour la table `aquanetmodule`
--
ALTER TABLE `aquanetmodule`
  MODIFY `moduleid` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT pour la table `aquanetstat`
--
ALTER TABLE `aquanetstat`
  MODIFY `statid` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT pour la table `aquanetuser`
--
ALTER TABLE `aquanetuser`
  MODIFY `userid` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT pour la table `logs`
--
ALTER TABLE `logs`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
